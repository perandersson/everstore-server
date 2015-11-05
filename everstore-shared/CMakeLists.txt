if(WIN32)
	set(socket_files win32/Win32Socket.cpp win32/Win32Socket.h win32/Win32Process.h win32/Win32Process.cpp win32/Win32Mutex.h win32/Win32Mutex.cpp)
else()
	set(socket_files gcc/GCCSocket.cpp gcc/GCCSocket.h gcc/GCCProcess.h gcc/GCCProcess.cpp gcc/GCCMutex.h gcc/GCCMutex.cpp)
endif()

add_library(everstore-shared ChildProcessId.h TransactionId.h ESErrorCodes.cpp ESErrorCodes.h ESRequestType.cpp ESRequestType.h ESHeader.h ESHeader.cpp Socket.cpp Socket.h Process.h FileLock.cpp FileLock.h IpcChildProcess.cpp IpcChildProcess.h IpcHost.cpp IpcHost.h IpcChild.cpp IpcChild.h Properties.cpp Properties.h StringUtils.h Bytes.cpp Bytes.h Event.h es_config.h everstore.h FileInputStream.cpp FileInputStream.h FileOutputStream.cpp FileOutputStream.h FileUtils.cpp FileUtils.h Journal.cpp Journal.h Messages.h Timestamp.cpp Timestamp.h Transaction.cpp Transaction.h LinkedList.h ByteBufferInputStream.cpp ByteBufferInputStream.h AutoClosable.h Mutex.h ${socket_files})
