#include "TauCOM.hpp"
#include "TauCOM.impl.hpp"
#include <Console.hpp>
#include <ConPrinter.hpp>
#include <String.hpp>
#include <allocator/TauAllocator.hpp>

namespace tau::com {

class IConsolePrinter : public IUnknown
{
public:
    struct ConstructionInfo final : public BaseConstructionInfo
    {
        DEFAULT_CONSTRUCT_PU(ConstructionInfo);
        DEFAULT_CM_PU(ConstructionInfo);
        DEFAULT_DESTRUCT(ConstructionInfo);
    public:
        C8DynString PrefixString;
    };
public:
    virtual void Print(const C8DynString& str) noexcept = 0;
};

class IConsoleLinePrinter : public IUnknown
{
public:
    using ConstructionInfo = IConsolePrinter::ConstructionInfo;
public:
    virtual void PrintLn(const C8DynString& str) noexcept = 0;
};

ResultCode RegisterConsolePrinter(IComManager* const comManager) noexcept;

}

TAU_DECL_UUID(IConsolePrinter, 0xBCE8AB2D7BC458BEull, 0xED4A89E7B2284D6Bull);
TAU_DECL_UUID(IConsoleLinePrinter, 0x98716AFD4475A23Cull, 0xB8565C0769E9454Eull);

static void PrintWorld(::tau::com::IConsoleLinePrinter* printer) noexcept
{
    printer->PrintLn(u8" World!");
}

int main(int argCount, char* args[]) 
{
    using namespace tau::com;

    Console::Init();

    IComManager* comManager;

    ConPrinter::PrintLn("Getting ComManager.");

    ResultCode status = TauComGetComManager(&comManager);

    if(!IsSuccess(status))
    {
        ConPrinter::PrintLn("Failed to get global ComManager.");
        return -101;
    }

    ConPrinter::PrintLn("Registering ConsolePrinter.");

    status = RegisterConsolePrinter(comManager);
    if(!IsSuccess(status))
    {
        ConPrinter::PrintLn("Failed to register ConsolePrinter.");
        return -201;
    }

    IConsolePrinter::ConstructionInfo printerConstructionInfo { };
    printerConstructionInfo.Iid = iid_of<IConsolePrinter>;
    printerConstructionInfo.pNext = nullptr;
    printerConstructionInfo.PrefixString = u8"";

    ConPrinter::PrintLn("Creating IConsolePrinter.");

    IConsolePrinter* printer;
    status = comManager->CreateObject<IConsolePrinter>(&printer, &printerConstructionInfo);

    if(!IsSuccess(status))
    {
        ConPrinter::PrintLn("Failed to create ConsolePrinter.");
        return -103;
    }

    printer->Print(u8"Hello");

    ComRef<IConsoleLinePrinter> linePrinter;
    printer->QueryInterface<IConsoleLinePrinter>(linePrinter.Load());

    PrintWorld(linePrinter);


    linePrinter = nullptr;
    printer->ReleaseReference();

    printerConstructionInfo.PrefixString = u8"> ";

    ConPrinter::PrintLn("Creating IConsolePrinter with prefix.");

    status = comManager->CreateObject<IConsolePrinter>(&printer, &printerConstructionInfo);

    if(!IsSuccess(status))
    {
        ConPrinter::PrintLn("Failed to create ConsolePrinter.");
        return -103;
    }

    printer->Print(u8"Hello");

    printer->QueryInterface<IConsoleLinePrinter>(linePrinter.Load());

    linePrinter->PrintLn(u8" World!");
    
    printer->ReleaseReference();

    linePrinter->PrintLn(u8"Hello World!");

    return 0;
}


namespace tau::com {

class ConsolePrinter final : public IConsolePrinter, public IConsoleLinePrinter
{
    DEFAULT_CONSTRUCT_PU(ConsolePrinter);
    DEFAULT_CM_PU(ConsolePrinter);
    DEFAULT_DESTRUCT(ConsolePrinter);
TAU_COM_IMPL_REF_COUNT();
public:
    ConsolePrinter(const C8DynString& prefixString) noexcept
        : m_PrefixString(prefixString)
    { }

    ResultCode QueryInterface(const UUID& iid, void** const pInterface) noexcept override
    {
        if(!pInterface)
        {
            return RC_NullParam;
        }

        if(iid == iid_of<IUnknown> || iid == iid_of<IConsolePrinter>)
        {
            *pInterface = static_cast<IConsolePrinter*>(this);
        }
        else if(iid == iid_of<IConsoleLinePrinter>)
        {
            *pInterface = static_cast<IConsoleLinePrinter*>(this);
        }
        else
        {
            return RC_InterfaceNotFound;
        }

        AddReference();
        return RC_Success;
    }

    void Print(const C8DynString& str) noexcept override
    {
        if(m_PrefixString)
        {
            ConPrinter::Print("{}", m_PrefixString);
        }

        ConPrinter::Print("{}", str);
    }

    void PrintLn(const C8DynString& str) noexcept override
    {
        if(m_PrefixString)
        {
            ConPrinter::Print("{}", m_PrefixString);
        }

        ConPrinter::PrintLn("{}", str);
    }
public:
    static ResultCode Factory(const UUID& iid, void** const pInterface, const BaseConstructionInfo* const pConstructionInfo) noexcept
    {
        if(!pInterface)
        {
            return RC_NullParam;
        }

        if(!pConstructionInfo)
        {
            return RC_NullParam;
        }

        if(iid == iid_of<IConsolePrinter> && pConstructionInfo->Iid == iid_of<IConsolePrinter>)
        {
            *pInterface = static_cast<IConsolePrinter*>(BasicTauAllocator<AllocationTracking::None>::Instance().AllocateT<ConsolePrinter>(static_cast<const IConsolePrinter::ConstructionInfo*>(pConstructionInfo)->PrefixString));
        }
        else if(iid == iid_of<IConsoleLinePrinter> && pConstructionInfo->Iid == iid_of<IConsoleLinePrinter>)
        {
            *pInterface = static_cast<IConsoleLinePrinter*>(BasicTauAllocator<AllocationTracking::None>::Instance().AllocateT<ConsolePrinter>(static_cast<const IConsoleLinePrinter::ConstructionInfo*>(pConstructionInfo)->PrefixString));
        }
        else
        {
            return RC_InterfaceNotFound;
        }

        return RC_Success;
    }
private:
    C8DynString m_PrefixString;
};

ResultCode RegisterConsolePrinter(IComManager* const comManager) noexcept
{
    if(!comManager)
    {
        return RC_NullParam;
    }

    ResultCode result = comManager->RegisterIidFactory(iid_of<IConsolePrinter>, ConsolePrinter::Factory);

    if(IsFailure(result) && result != RC_FactoryAlreadyRegistered)
    {
        return result;
    }

    result = comManager->RegisterIidFactory(iid_of<IConsoleLinePrinter>, ConsolePrinter::Factory);

    return result;
}

}

