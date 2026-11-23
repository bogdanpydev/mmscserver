//---------------------------------------------------------------------------

#ifndef thread2H
#define thread2H
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class Thread2 : public TThread
{            
private:
protected:
        void __fastcall Execute();
public:
        __fastcall Thread2(bool CreateSuspended);
        bool success;
};
//---------------------------------------------------------------------------
#endif
 