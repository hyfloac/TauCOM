#include "TauCOM.hpp"
#include "TauCOM.impl.hpp"
#include <allocator/TauAllocator.hpp>

namespace tau::com {

class ComManager final : public IComManager1
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

    // IUnknown
    ResultCode QueryInterface(const UUID& iid, void** const pInterface) noexcept override;

    // IComManager
    ResultCode RegisterIidFactory(const UUID& iid, const ComFactoryFunc factory) noexcept override;
    ResultCode CreateObject(const UUID& iid, void** const pInterface, const BaseConstructionInfo* const pConstructionInfo) noexcept override;

    // IComManager1
    ResultCode UnregisterIidFactory(const UUID& iid) noexcept override;
    ResultCode GetIidFactory(const UUID& iid, ComFactoryFunc* const factory) noexcept override;
    ResultCode Duplicate(IComManager1** const comManager) noexcept override;
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

    if(iid == iid_of<IUnknown> || iid == iid_of<IComManager> || iid == iid_of<IComManager1>)
    {
        *pInterface = static_cast<IComManager1*>(this);
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

ResultCode ComManager::UnregisterIidFactory(const UUID& iid) noexcept
{
    if(!m_Factories.contains(iid))
    {
        return RC_InterfaceNotFound;
    }

    (void) m_Factories.erase(iid);

    return RC_Success;
}

ResultCode ComManager::GetIidFactory(const UUID& iid, ComFactoryFunc* const factory) noexcept
{
    if(!m_Factories.contains(iid))
    {
        *factory = nullptr;
        return RC_InterfaceNotFound;
    }

    *factory = m_Factories[iid];

    return RC_Success;
}

ResultCode ComManager::Duplicate(IComManager1** const comManager) noexcept
{
    ConstructionInfo constructionInfo;
    constructionInfo.Iid = iid_of<IComManager1>;
    constructionInfo.pNext = nullptr;
    constructionInfo.Factories = m_Factories;

    return IComManager::CreateObject<IComManager1>(comManager, &constructionInfo);
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
        if(pConstructionInfo->Iid != iid_of<IComManager> && pConstructionInfo->Iid != iid_of<IComManager1>)
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
        factories[iid_of<IComManager1>] = ComManager::Factory;

        s_GlobalComManager = BasicTauAllocator<AllocationTracking::None>::Instance().AllocateT<ComManager>(::std::move(factories));
    }

    *pInterface = s_GlobalComManager;

    return RC_Success;
}
