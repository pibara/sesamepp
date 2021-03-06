#include "openfilecollection.hpp"
#include <iostream>
#include <vector>
class Node {
    int mNum;
    bool mValid;
    bool mOpen;
  public:
    Node(Node const &) = delete;
    Node & operator=(Node const & n) = delete; 
    Node& operator=(Node&& n) = delete;
    Node():mNum(0),mValid(false),mOpen(false){}
    /*Constructor for the null node*/
    Node(int num):mNum(num),mValid(true),mOpen(false){
        std::cerr << "constructor " << num << std::endl;
    }
    /*Move constructor*/
    Node(Node&& n):mNum(n.mNum),mValid(n.mValid),mOpen(n.mOpen){
        if (&n != this) {
          if (mValid) {
            n.mValid=false;
          } else {
            std::cerr << "BOGUS use of move constructor " << mNum << std::endl;
          }
        } else {
           std::cerr << "BOGUS self assignment in move constructor " << mNum << std::endl;
        }
    }
    /*Destructor*/
    ~Node() {
      if (mNum) {
        if (mValid) {
          if (mOpen) {
             std::cerr << "Destructor of open node " << mNum << std::endl;
             this->lowLevelClose();
          } else {
              std::cerr << "Destructor of closed node " << mNum << std::endl;
          }
        }
      }
    }
    void lowLevelClose() { 
       if (mValid) {
         if (mOpen) {
           std::cerr << "lowLevelClose of open node " <<  mNum << std::endl;
           mOpen=false;
         } else {
           std::cerr << "lowLevelClose of closed node " <<  mNum << std::endl;
         }
       } else {
          std::cerr << "BOGUS lowLevelClose of invalidated node" <<  mNum << std::endl;
       }
    }
    void lowLevelOpen() {
       if (mValid) {
         if (mOpen) {
           std::cerr << "BOGUS lowLevelOpen of open node " <<  mNum << std::endl;
         } else {
           std::cerr << "lowLevelOpen " <<  mNum << std::endl;
           mOpen=true;
         }
       } else {
          std::cerr << "BOGUS lowLevelOpen of invalidated node" <<  mNum << std::endl;
       }
    }
};

int main(int argc,char **argv) {
   openfilecollection<Node,4UL,1000UL> coll;
   std::vector<uint64_t> fhs(10);
   for (int index=0; index < 10; ++index) {
      fhs[index]=coll.open(index + 1);
   }
   auto n1=coll[fhs[0]];
   auto n2=coll[fhs[2]];
   auto n3=coll[fhs[4]];
   auto n4=coll[fhs[6]];
   auto n5=coll[fhs[8]];
   coll.close(fhs[0]);
   coll.close(fhs[9]);
   coll.close(fhs[2]);
   coll.close(fhs[8]);
   coll.close(fhs[3]);
   return 0;
}
