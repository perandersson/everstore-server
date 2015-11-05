#ifndef _EVERSTORE_LINKED_LIST_H_
#define _EVERSTORE_LINKED_LIST_H_

#include "es_config.h"

template<class T> class LinkedListLink;
template<class T> class LinkedList;

template<class T>
class LinkedListLink
{
public:
	//
	// Constructor
	LinkedListLink();

	//
	// Destructor
	virtual ~LinkedListLink();

	//
	// Unlink this node from the linked list.
	void unlink();

	//
	// Link an item
	void link(T* item, LinkedList<T>* list);

	//
	// @return TRUE if this link is attached to a list.
	bool isLinked() const;

private:
	size_t mOffset;

public:
	T* head;
	T* tail;
	LinkedList<T>* list;
};

template<class T>
class LinkedList
{
	friend class LinkedListLink<T>;

public:
	typedef LinkedListLink<T> Link;

public:
	//
	// Constructor
	LinkedList(size_t offset);

	//
	// Destructor
	~LinkedList();

	// 
	// Add an item at the beginning of the list
	// @param item the item we want to add into this list
	void addFirst(T* item);

	// 
	// Add an item at the end of the list
	// @param item the item we want to add into this list
	void addLast(T* item);

	//
	// Removes an item from the list
	void remove(T* item);

	//
	// Returns the first element of this list.
	T* first() const;

	//
	// Returns the last element of this list
	T* last() const;

	//
	// Move the supplied item to the end of this list
	void moveToLast(T* item);

	// 
	// Delete all nodes inside this list
	void deleteAll();
	
	//
	// Unlinks all the nodes inside this list
	void unlinkAll();

	//
	// @return The number of elements located in this list
	uint32_t getSize() const;

	//
	// Check to see if this list is empty or not
	bool empty() const { return getSize() == 0; }

protected:
	//
	// Retrieves the link value from the supplied item.
	Link* getLink(T* item);
		
private:
	size_t mLinkOffset;
	T* mHead;
	T* mTail;
	uint32_t mSize;
};

/////////////////////////////////

template<class T>
LinkedListLink<T>::LinkedListLink() : mOffset(0), head(NULL), tail(NULL), list(NULL)
{
}

template<class T>
LinkedListLink<T>::~LinkedListLink()
{
	unlink();
}

template<class T>
void LinkedListLink<T>::unlink()
{
	if (list != NULL) {
		// We are head if Head is NULL!
		if (head == NULL) {
			list->mHead = tail;
		}
		
		// We are tail if Tail is NULL!
		if (tail == NULL) {
			list->mTail = head;
		}

		if (head != NULL) {
			LinkedListLink<T>* link = (LinkedListLink<T>*)((char*)(head) + mOffset);
			link->tail = tail;
		}

		if (tail != NULL) {
			LinkedListLink<T>* link = (LinkedListLink<T>*)((char*)(tail)+mOffset);
			link->head = head;
		}

		list->mSize--;
	}

	list = NULL;
	head = NULL;
	tail = NULL;
}

template<class T>
void LinkedListLink<T>::link(T* item, LinkedList<T>* _list)
{
	// Offset is needed so that we can find where the link is located inside the item
	mOffset = (char*)(this) - (char*)(item);
	list = _list;
}

template<class T>
bool LinkedListLink<T>::isLinked() const
{
	return list != NULL;
}

///////////////////////////////////

template<class T>
LinkedList<T>::LinkedList(size_t offset) : mLinkOffset(offset), mHead(NULL), mTail(NULL), mSize(0)
{
}

template<class T>
LinkedList<T>::~LinkedList()
{
	unlinkAll();
}

template<class T>
void LinkedList<T>::addFirst(T* item)
{
	// Find the link for the supplied item
	Link* link = getLink(item);

	// Make sure that the items link isn't attached to the list
	link->unlink();

	// Assign the item into the linked list
	if (mHead == NULL) {
		mHead = mTail = item;
	}
	else {
		// Put the item to the end of the list
		getLink(mHead)->head = item;
		link->tail = mHead;
		mHead = item;
	}

	// Link the item with this list
	link->link(item, this);
	mSize++;
}

template<class T>
void LinkedList<T>::addLast(T* item)
{
	// Find the link for the supplied item
	Link* link = getLink(item);
		
	// Make sure that the items link isn't attached to the list
	link->unlink();

	// Assign the item into the linked list
	if(mHead == NULL) {
		mHead = mTail = item;
 	} else {
		// Put the item to the end of the list
		getLink(mTail)->tail = item;
		link->head = mTail;
		mTail = item;
	}

	// Link the item with this list
	link->link(item, this);
	mSize++;
}

template<class T>
void LinkedList<T>::remove(T* item) 
{
	assert(item != NULL && "You cannot remove a non-existing item");

	Link* link = getLink(item);
	if(link->list == NULL)
		return;

	assert(link->list == this && "You cannot remove another lists nodes");

	link->unlink();
}

template<class T>
typename LinkedList<T>::Link* LinkedList<T>::getLink(T* item)
{
	char* mem = reinterpret_cast<char*>(item) + mLinkOffset;
	return reinterpret_cast<Link*>(mem);
}

template<class T>
T* LinkedList<T>::first() const
{
	return mHead;
}

template<class T>
T* LinkedList<T>::last() const
{
	return mTail;
}

template<class T>
void LinkedList<T>::moveToLast(T* item)
{
	remove(item);
	addLast(item);
}

template<class T>
void LinkedList<T>::unlinkAll() {
	T* ptr = mHead;
	while(ptr != NULL) {
		Link* link = getLink(ptr);
		T* next = link->tail;
		link->head = link->tail = NULL;
		link->list = NULL;
		ptr = next;
	}
	mHead = mTail = NULL;
}

template<class T>
void LinkedList<T>::deleteAll()
{
	T* ptr = mHead;
	while(ptr != NULL) {
		Link* link = getLink(ptr);
		T* next = link->tail;
		delete ptr;
		ptr = next;
	}
	mHead = mTail = NULL;
}

template<class T>
uint32_t LinkedList<T>::getSize() const
{
	return mSize;
}

#endif
