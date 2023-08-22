#include "TauCOM.hpp"
#include "TauCOM.impl.hpp"
#include <allocator/TauAllocator.hpp>

namespace tau::com {

class ComManager final : public IComManager
{
    DEFAULT_CONSTRUCT_PU(ComManager);
    DEFAULT_DESTRUCT(ComManager);
    TAU_COM_IMPL_REF_COUNT();
public:
    ComManager(const FactoryMap& factories) noexcept;
    ComManager(FactoryMap&& factories) noexcept;

    inline ComManager(const ComManager& copy) noexcept;
    inline ComManager(ComManager&& move) noexcept;
    inline ComManager& operator=(const ComManager& copy) noexcept;
    inline ComManager& operator=(ComManager&& move) noexcept;

    ResultCode QueryInterface(const UUID& iid, void** const pInterface) noexcept override;
    ResultCode RegisterIidFactory(const UUID& iid, const ComFactoryFunc factory) noexcept override;
    ResultCode CreateObject(const UUID& iid, void** const pInterface, const BaseConstructionInfo* const pConstructionInfo) noexcept override;
public:
    static ResultCode Factory(const UUID& iid, void** const pInterface, const BaseConstructionInfo* const pConstructionInfo) noexcept;
private:
    FactoryMap m_Factories;
};

ComManager::ComManager(const FactoryMap& factories) noexcept
    : m_Factories(factories)
{ }

ComManager::ComManager(FactoryMap&& factories) noexcept
    : m_Factories(::std::move(factories))
{ }

ComManager::ComManager(const ComManager& copy) noexcept
    : m_Factories(copy.m_Factories)
{ }

ComManager::ComManager(ComManager&& move) noexcept
    : m_Factories(::std::move(move.m_Factories))
{ }

ComManager& ComManager::operator=(const ComManager& copy) noexcept
{
    if(this == &copy)
    {
        return *this;
    }

    m_Factories = copy.m_Factories;

    return *this;
}

ComManager& ComManager::operator=(ComManager&& move) noexcept
{
    if(this == &move)
    {
        return *this;
    }

    m_Factories = ::std::move(move.m_Factories);

    return *this;
}

ResultCode ComManager::QueryInterface(const UUID& iid, void** const pInterface) noexcept
{
    if(!pInterface)
    {
        return RC_NullParam;
    }

    if(iid == iid_of<IUnknown> || iid == iid_of<IComManager>)
    {
        *pInterface = static_cast<IComManager*>(this);
    }
    else
    {
        return RC_InterfaceNotFound;
    }

    AddReference();
    return RC_Success;
}

ResultCode ComManager::RegisterIidFactory(const UUID& iid, const ComFactoryFunc factory) noexcept
{
    if(!factory)
    {
        return RC_NullParam;
    }

    ResultCode ret = RC_Success;

    if(m_Factories.contains(iid))
    {
        ret = RC_FactoryAlreadyRegistered;
    }

    m_Factories[iid] = factory;

    return ret;
}

ResultCode ComManager::CreateObject(const UUID& iid, void** const pInterface, const BaseConstructionInfo* const pConstructionInfo) noexcept
{
    if(!m_Factories.contains(iid))
    {
        return RC_InterfaceNotFound;
    }

    return m_Factories[iid](iid, pInterface, pConstructionInfo);
}

ResultCode ComManager::Factory(const UUID& iid, void** const pInterface, const BaseConstructionInfo* const pConstructionInfo) noexcept
{
    if(!pInterface)
    {
        return RC_NullParam;
    }

    if(iid != iid_of<IComManager>)
    {
        return RC_InterfaceNotFound;
    }

    if(pConstructionInfo)
    {
        if(pConstructionInfo->Iid != iid_of<IComManager>)
        {
            return RC_InterfaceNotFound;
        }

        const ConstructionInfo* const constructionInfo = static_cast<const ConstructionInfo*>(pConstructionInfo);

        *pInterface = BasicTauAllocator<AllocationTracking::None>::Instance().AllocateT<ComManager>(constructionInfo->Factories);
    }
    else
    {
        *pInterface = BasicTauAllocator<AllocationTracking::None>::Instance().AllocateT<ComManager>();
    }

    return RC_Success;
}

static ComManager* s_GlobalComManager = nullptr;

}

extern "C" TAU_COM_LIB::tau::com::ResultCode TauComGetComManager(::tau::com::IComManager** const pInterface) noexcept
{
    using namespace tau::com;

    if(!pInterface)
    {
        return RC_NullParam;
    }

    if(!s_GlobalComManager)
    {

        ComManager::FactoryMap factories;
        factories[iid_of<IComManager>] = ComManager::Factory;

        s_GlobalComManager = BasicTauAllocator<AllocationTracking::None>::Instance().AllocateT<ComManager>(::std::move(factories));
    }

    *pInterface = s_GlobalComManager;

    return RC_Success;
}
