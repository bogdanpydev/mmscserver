//---------------------------------------------------------------------------

#ifndef thread1H
#define thread1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class thread1 : public TThread
{            
private:
protected:
        void __fastcall Execute();
       
public:
        __fastcall thread1(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
