#include "GCCProcess.h"
#include <sys/un.h>
// This line is neccessary for GNUGCC to work
#include "../Socket.h"
#include "../Message/ESHeader.h"
#include "GCCMutex.h"

string PIPE_NAME_PREFIX("everstore_pipe_child");

ESErrorCode _gcc_pipe_open(const string& name, int* pipe) {
	unlink(name.c_str());
	const string pipeName = name;
	struct sockaddr_un addr;
	SOCKET fd;
	*pipe = INVALID_PIPE;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		return ESERR_PIPE_CREATE;
	}

	socket_setblocking(fd);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	unlink(pipeName.c_str());
	strcpy(addr.sun_path, pipeName.c_str());

	if (::bind(fd, (struct sockaddr*) &(addr), sizeof(addr)) < 0) {
		socket_close(fd);
		return ESERR_PIPE_LISTEN;
	}

	if (::listen(fd, 1) < 0) {
		socket_close(fd);
		return ESERR_PIPE_LISTEN;
	}

	*pipe = fd;
	return ESERR_NO_ERROR;
}

void _gcc_pipe_close(int* p) {
	if (*p != INVALID_PIPE) {
		socket_close(*p);
		*p = INVALID_PIPE;
	}
}

void process_init(process_t* p) {
	p->handle = INVALID_PROCESS;
	p->pipe = INVALID_PIPE;
}

ESErrorCode process_start(const string& asd, const string& command, const string& currentDirectory,
		const vector<string>& arguments, uint32_t pipeMaxBufferSize, process_t* p) {
	const auto pipeName = PIPE_NAME_PREFIX + name;
	// Open the pipe first
	ESErrorCode err = _gcc_pipe_open(pipeName, &p->pipe);
	if (isError(err)) {
		return err;
	}

	const char* argv[arguments.size() + 2];
	char directory[1024];
	getcwd(directory, sizeof(directory));
	const string c = string(directory) + string("/") + command;
	argv[0] = c.c_str();
	for(auto i = 0; i < arguments.size(); ++i) {
		argv[i + 1] = arguments[i].c_str();
	}
	argv[arguments.size() + 1] = NULL;

	// Duplicate this process
	pid_t pid = fork();
	if (pid != 0) {
		p->handle = pid;
	}
	else {
		// Replace this forked process with the executable file
		printf("Starting command: %s\n", c.c_str());
		auto result = execvp(c.c_str(), (char**)argv);
		printf("Command failure: %d\n", result);
		abort();
	}

	SOCKET pipe = p->pipe;
	p->pipe = socket_accept_blocking(pipe, pipeMaxBufferSize);
	socket_close(pipe);
	return p->pipe < 0 ? ESERR_PIPE_CONNECT : ESERR_NO_ERROR;
}

ESErrorCode process_close(process_t* p) {
	if (p->pipe != INVALID_PIPE) {
		_gcc_pipe_close(&p->pipe);
		p->pipe = INVALID_PIPE;
	}
	if (p->handle != INVALID_PROCESS) {
		killpg(p->handle, SIGKILL);
		p->handle = INVALID_PROCESS;
	}
	return ESERR_NO_ERROR;
}

ESErrorCode process_wait(process_t* p) {
	int ret = 0;
	waitpid(p->handle, &ret, 0);
	return ESERR_NO_ERROR;
}

ESErrorCode process_connect_to_host(const string& name, process_t* p) {
	const auto pipeName = PIPE_NAME_PREFIX + name;
	struct sockaddr_un addr;
	SOCKET fd;
	p->handle = INVALID_PROCESS;
	p->pipe = INVALID_PIPE;

	if ((fd = ::socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		return ESERR_PIPE_CONNECT;
	}

	socket_setblocking(fd);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, pipeName.c_str());

	if (::connect(fd, (struct sockaddr*) &(addr), sizeof(addr)) < 0) {
		socket_close(fd);
		return ESERR_PIPE_CONNECT;
	}

	p->pipe = fd;
	return p->pipe < 0 ? ESERR_PIPE_CONNECT : ESERR_NO_ERROR;
}

uint32_t process_read(process_t* p, char* bytes, uint32_t size) {
	return socket_recvall(p->pipe, bytes, size);
}

uint32_t process_write(process_t* p, const char* bytes, uint32_t size) {
	return socket_sendall(p->pipe, bytes, size);
}

ESErrorCode _gcc_socket_share(process_t* p, SOCKET hostSocket, mutex_t hostSocketLock) {
	struct msghdr message;
	struct iovec iov[1];
	struct cmsghdr *control_message = NULL;
	char ctrl_buf[CMSG_SPACE(sizeof(int))];
	char data[1];

	assert(hostSocketLock != nullptr);

	memset(&message, 0, sizeof(struct msghdr));
	memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

	// We are passing at least one byte of data so that recvmsg() will not return 0
	data[0] = ' ';
	iov[0].iov_base = data;
	iov[0].iov_len = sizeof(data);

	message.msg_name = NULL;
	message.msg_namelen = 0;
	message.msg_iov = iov;
	message.msg_iovlen = 1;
	message.msg_controllen =  CMSG_SPACE(sizeof(int));
	message.msg_control = ctrl_buf;

	control_message = CMSG_FIRSTHDR(&message);
	control_message->cmsg_level = SOL_SOCKET;
	control_message->cmsg_type = SCM_RIGHTS;
	control_message->cmsg_len = CMSG_LEN(sizeof(int));

	*((int *) CMSG_DATA(control_message)) = hostSocket;

	ssize_t len = sendmsg(p->pipe, &message, 0);
	if (len <= 0) {
		return ESSER_PROCESS_SHARE_SOCKET;
	}

	printf("Writing mutex to child process with socket lock name: %s\n", hostSocketLock->name);
	const uint32_t nameSize = sizeof(hostSocketLock->name);
	const uint32_t sentNameSize = process_write(p, hostSocketLock->name, nameSize);
	if (sentNameSize != nameSize) {
		printf("Could not write mutex to child process. Tried to send size: %d but sent %d\n", nameSize, sentNameSize);
		return ESSER_PROCESS_SHARE_SOCKET;
	}
	printf("Wrote mutex to child process\n");

	return ESERR_NO_ERROR;
}

ESErrorCode process_share_socket(process_t* p, SOCKET hostSocket, mutex_t hostSocketLock) {
	ESErrorCode err;
	const ESHeader header(REQ_NEW_CONNECTION, 0, 0, FALSE, hostSocket);
	if (process_write(p, (char*)&header, sizeof(header)) == 0)
		return ESSER_PROCESS_SHARE_SOCKET;

	err = _gcc_socket_share(p, hostSocket, hostSocketLock);
	if (isError(err)) {
		socket_close(hostSocket);
		return err;
	}

	return ESERR_NO_ERROR;
}

SOCKET _gcc_socket_attach(process_t* p, mutex_t* lock) {
	struct msghdr message;
	struct iovec iov[1];
	struct cmsghdr *control_message = NULL;
	char ctrl_buf[CMSG_SPACE(sizeof(int))];
	char data[1] = {' '};
	int res;

	memset(&message, 0, sizeof(struct msghdr));
	memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

	/* For the dummy data */
	iov[0].iov_base = data;
	iov[0].iov_len = sizeof(data);

	message.msg_name = NULL;
	message.msg_namelen = 0;
	message.msg_control = ctrl_buf;
	message.msg_controllen = CMSG_SPACE(sizeof(int));
	message.msg_iov = iov;
	message.msg_iovlen = 1;

	if((res = recvmsg(p->pipe, &message, 0)) <= 0) {
		socket_close(p->pipe);
		return res;
	}

	printf("Reading mutex from client\n");
	mutex_t mutexMem = (mutex_t) malloc(sizeof(_mutex_t));
	const uint32_t nameSize = sizeof(mutexMem->name);
	memset(mutexMem->name, 0, nameSize);
	if(process_read(p, mutexMem->name, nameSize) != nameSize) {
		printf("Could not read mutex data from process\n");
		return INVALID_SOCKET;
	}

	if (mutex_attach(string(mutexMem->name), mutexMem) != ESERR_NO_ERROR) {
		printf("Could not attach mutex from host process.\n");
		return INVALID_SOCKET;
	}
	printf("Read mutex from client\n");
	*lock = mutexMem;

	// Iterate through header to find if there is a file descriptor
	for(control_message = CMSG_FIRSTHDR(&message);
	    control_message != NULL;
	    control_message = CMSG_NXTHDR(&message,
	                                  control_message))
	{
		if( (control_message->cmsg_level == SOL_SOCKET) &&
		    (control_message->cmsg_type == SCM_RIGHTS) )
		{
			return *((int *) CMSG_DATA(control_message));
		}
	}



	return INVALID_SOCKET;
}

SOCKET process_accept_shared_socket(process_t* p, mutex_t* lock) {
	*lock = INVALID_LOCK;
	return _gcc_socket_attach(p, lock);
}

