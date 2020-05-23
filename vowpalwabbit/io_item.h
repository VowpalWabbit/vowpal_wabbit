#ifndef _IO_ITEM_
#define _IO_ITEM_
class IO_Item {

  public:

      std::string message;
      int numCharsInit;

      IO_Item(){
          numCharsInit = 0;
      }

      IO_Item(std::string myMsg, int myNumCharsInit){

          message.assign(myMsg);
          numCharsInit = myNumCharsInit;
      }

      IO_Item operator=(const IO_Item &toCopy){
          message.assign(toCopy.message);
          numCharsInit = toCopy.numCharsInit;
          return *this;
      }

      IO_Item(const IO_Item &toCopy){
          message.assign(toCopy.message);
          numCharsInit = toCopy.numCharsInit;
      }

      ~IO_Item() {}

      inline std::string getString(){
          return std::string(message);
      }

      inline int getNumCharsInit(){
        return numCharsInit;
      }

      inline void setString(std::string newMsg){
          message.assign(newMsg);
      }

      inline void setNumCharsInit(int newNum){
          numCharsInit = newNum;
      }

};

#endif
