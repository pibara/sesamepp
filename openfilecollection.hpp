//  Copyright (c) 2013, Rob J Meijer
//
//Permission is hereby granted, free of charge, to any person or organization
//obtaining a copy of the software and accompanying documentation covered by
//this license (the "Software") to use, reproduce, display, distribute,
//execute, and transmit the Software, and to prepare derivative works of the
//Software, and to permit third-parties to whom the Software is furnished to
//do so, all subject to the following:
//
//The copyright notices in the Software and this entire statement, including
//the above license grant, this restriction and the following disclaimer,
//must be included in all copies of the Software, in whole or in part, and
//all derivative works of the Software, unless such copies or derivative
//works are solely in the form of machine-executable object code generated by
//a source language processor.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
//SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
//FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//DEALINGS IN THE SOFTWARE.
#ifndef OPENFILECOLLECTION_HPP
#define OPENFILECOLLECTION_HPP
#include <map>
#include <deque>
#include <stdint.h>
#include <sys/types.h>
//This template is meant as a generic container for node objects.
//A node object needs to have the following zero argument void methods defined:
//  * lowLevelClose()  
//  * lowLevelOpen()
//The template takes 3 arguments:
//  * The type of the node to be used.
//  * The maximum number of low level file objects that we can keep open at the same time.
//  * The maximum size of the internal event queue.
template <typename nodeType,size_t maxOpenFiles,size_t maxQueueSize>
class openfilecollection {
    nodeType mNull;
    uint64_t mLastHandle; //Used for determinign the unique file handle. 
    std::map<uint64_t, nodeType> mCollection; //The collection ov virtually open file nodes.
    std::map<uint64_t, size_t> mFullyOpen; //A map holding the event count for each low-level open file node mentioned 
                                           //in the event queue.
    std::deque<uint64_t> mOpperQue;        //An event queue that is used to determine what virtualy open node may be low-level
                                           //closed next when needed.
    //Method for getting the next free high-level file handle
    uint64_t  getFreeFhNumber() {
       //0 isn't a valid file handle, other numbers are valid as long as they are not yet issued.
       while ((mLastHandle == 0) || (mCollection.count(mLastHandle))) {
         mLastHandle += 1;
       }
       return mLastHandle;
    }
    
    //This method will remove events from the front of the event queue when possible or needed.
    void cleanupQueFront() {
        //If the front most node is in the queue more often than once, we can savely remove it and adjust
        //the count in mFullyOpen. If the size of the queue is larger than the maximum size, we shall remove it
        //even if its not needed from a number of open file handles perspective.
        while ((mFullyOpen.count(mOpperQue.front()) == 0)||(mFullyOpen[mOpperQue.front()]>1)||(mOpperQue.size()>maxQueueSize)) {
            if (mFullyOpen.count(mOpperQue.front()) != 0) {
              mFullyOpen[mOpperQue.front()] -= 1; //Reduce the count.
              if (mFullyOpen[mOpperQue.front()] == 0) { //If this means the count drops to zero, low-level close the node for now.
                  mCollection[mOpperQue.front()].lowLevelClose();
                  mFullyOpen.erase(mOpperQue.front());
              }
            }
            mOpperQue.pop_front(); //Drop the event from the event queue.
            
        }
    }
    //This method will low-level close high-level opened file node's untill we drop below the 
    //maximum allowed number of open files.
    void tempCloseIfNeeded() {
        while (mFullyOpen.size() > maxOpenFiles) {
            uint64_t candidate = mOpperQue.front(); //First remember the fh of the event we are to remove from the queue.
            mOpperQue.pop_front(); //Now drop the entry from the queue.
            if (mFullyOpen.count(candidate)) { //Check if the removed fh was still to be considered to be low-level open.
              mFullyOpen[candidate] -= 1;  //Adjust the counter
              if (mFullyOpen[candidate] == 0) { //If the counter drops to 0, low level close the node.
                  mCollection[candidate].lowLevelClose();
                  mFullyOpen.erase(candidate);
              }
            }
        }
        this->cleanupQueFront(); //There may be some entries now at the end of the queue that can be cleaned up.
    }
    openfilecollection(openfilecollection const &) = delete;
    openfilecollection &operator=(openfilecollection const &) = delete;
  public:
    openfilecollection():mNull(),mLastHandle(0){} //Constructor initializes the mLastHandle value.
    ~openfilecollection() { 
       //Destructor will close any low level open file node.
       for (std::map<uint64_t, size_t>::iterator   i1=mFullyOpen.begin();  i1 != mFullyOpen.end(); ++i1) {
           uint64_t fh = i1->first;
           mCollection[fh].lowLevelClose();
       }
    }
    class node_handle {
        openfilecollection<nodeType, maxOpenFiles, maxQueueSize> &mCol;
        uint64_t mFh;
       public:
        node_handle(openfilecollection<nodeType, maxOpenFiles, maxQueueSize> &col, uint64_t fh):mCol(col),mFh(fh){}
        void close() {
           mCol->close(mFh);
        }
        ssize_t read(void *buf, size_t count,off_t offset) {
           return mCol->mCollection[mFh].read(buf,count);
        }
        ssize_t write(const void *buf, size_t count,off_t offset) {
           return mCol->mCollection[mFh].write(buf,count);
        }
        int chmod(mode_t mode) {
           return mCol->mCollection[mFh].chmod(mode);
        }
    };
    //Operator for accessing the file node object. This operator will return a low-level opened file object by handle.
    node_handle operator[](uint64_t fh) {
        if (mCollection.count(fh) == 0) { //If fh is not in map, return the default constructed null object.
            return node_handle(*this,0);
        }
        if (mFullyOpen.count(fh)) { //If the file node is already low-level open:
          if (fh != mOpperQue.back()) { // If adding the operation to the queue won't result in a duplicate:
            mFullyOpen[fh] += 1;   //Update the occurence count
            mOpperQue.push_back(fh);//And add an other occurence of this file handle to the back of the queu. 
          }
        } else { //If the file curently wasn't low-level opened:
          mFullyOpen[fh] = 1; //Set the occurence count to one.
          mOpperQue.push_back(fh); //Add the file handle to the event queu.
          this->tempCloseIfNeeded(); //and make sure we don't exeed the max number of open files.
          mCollection[fh].lowLevelOpen(); //Now do a low level open of the file node.
        }
        return node_handle(*this,fh); //Return our open file node as handle;
    }
    //This method adds a new node to the container and does both a high level and low level open.
    template<typename ... Args>
    uint64_t open(Args&& ... args) {
        uint64_t fh=this->getFreeFhNumber(); //Get a new fh number.
        mCollection.emplace(std::piecewise_construct,std::forward_as_tuple(fh),std::forward_as_tuple(args...));
        mFullyOpen[fh] = 1; //Set the queue occurence count to one.
        mOpperQue.push_back(fh); //and add the file handle to the event queue.
        this->tempCloseIfNeeded(); //Before opening the new file, make sure we don't exeed the maximum number of open files.
        mCollection[fh].lowLevelOpen(); //Do the low level file open.
        return fh; //Return our brand new high-level open file handle.
    }
    //Method for explicit high-level closing of a file node.
    void close(uint64_t fh) {
        //If the file is low-level open, close it first.
        if (mFullyOpen.count(fh)) {
            mFullyOpen.erase(fh);
            mCollection[fh].lowLevelClose();
        }
        //Drop from our high-level collection.
        if (mCollection.count(fh)) {
          mCollection.erase(fh);
        }
        //Note: there may still be entries in mOpperQue refering to the deleted entity, won't clean up here thus other
        //operations need resilience regarding dead file-handles.
        //All we do for now is cleanup the queue front, just in case it starts with the just erased handle.
        this->cleanupQueFront();
    }
};

#endif
