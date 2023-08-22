// ReSharper disable CppInconsistentNaming
#pragma once

#include <unordered_map>
#include <new>
#include <functional>

#include <TauMacros.hpp>
#include <NumTypes.hpp>
#include <Objects.hpp>

#ifdef TAU_COM_BUILD_SHARED
  #define TAU_COM_LIB DYNAMIC_EXPORT
#elif defined(TAU_COM_IMPORT_SHARED) || 1
  #define TAU_COM_LIB DYNAMIC_IMPORT
#else
  #define TAU_COM_LIB
#endif

namespace tau::com {

struct UUID final
{
public:
    u64 Low;
    u64 High;
public:
    constexpr UUID() noexcept = default;

    constexpr UUID(const u64 low, const u64 high) noexcept
        : Low(low)
        , High(high)
    { }

    constexpr ~UUID() noexcept = default;

    constexpr UUID(const UUID& copy) noexcept = default;
    constexpr UUID(UUID&& move) noexcept = default;

    constexpr UUID& operator=(const UUID& copy) noexcept = default;
    constexpr UUID& operator=(UUID&& move) noexcept = default;

    [[nodiscard]] constexpr bool operator ==(const UUID& other) const noexcept { return Low == other.Low && High == other.High; }
    [[nodiscard]] constexpr bool operator !=(const UUID& other) const noexcept { return !((*this) == other); }
};

}

namespace std {

template<>
struct hash<::tau::com::UUID>
{
    [[nodiscard]] ::std::size_t operator()(const ::tau::com::UUID& uuid) const noexcept
    {
        return uuid.Low ^ uuid.High;
    }
};

}

namespace tau::com {

template<typename T>
struct ComUUID final
{
    static inline constexpr UUID IID = UUID(0x0000000000000000ull, 0x0000000000000000ull);
};

#define TAU_DECL_UUID(T, LOW, HIGH) \
    template<> \
    struct ComUUID<T> final { \
        static inline constexpr UUID IID = UUID((LOW), (HIGH)); \
    }

template<typename T>
inline constexpr const UUID& uuid_of = ComUUID<T>::IID;

template<typename T>
inline constexpr const UUID& iid_of = ComUUID<T>::IID;


enum ResultCode : i32
{
    RC_Success = 0,
    RC_NullParam = -1,
    RC_InterfaceNotFound = -2,
    RC_FactoryAlreadyRegistered = 1
};

static bool IsSuccess(const ResultCode result) noexcept { return static_cast<i32>(result) >= 0; }
static bool IsFailure(const ResultCode result) noexcept { return !IsSuccess(result); }

struct BaseConstructionInfo
{
    DEFAULT_CONSTRUCT_PU(BaseConstructionInfo);
    DEFAULT_CM_PU(BaseConstructionInfo);
    DEFAULT_DESTRUCT_VI(BaseConstructionInfo);
public:
    UUID Iid;
    const BaseConstructionInfo* pNext;
};

class IUnknown
{
    DEFAULT_CONSTRUCT_PO(IUnknown);
    DEFAULT_CM_PO(IUnknown);
    DEFAULT_DESTRUCT_VI(IUnknown);
public:
    using ConstructionInfo = BaseConstructionInfo;
public:
    virtual i32 AddReference() noexcept = 0;
    virtual i32 ReleaseReference() noexcept = 0;

    virtual ResultCode QueryInterface(const UUID& iid, void** const pInterface) noexcept = 0;

    template<typename T>
    ResultCode QueryInterface(T** pInterface) noexcept
    {
        return QueryInterface(iid_of<T>, reinterpret_cast<void**>(pInterface));
    }
};

TAU_DECL_UUID(IUnknown, 0x89D0171D1E547699ull, 0x3513C89A25664A40ull);

class IComManager : public IUnknown
{
    DEFAULT_CONSTRUCT_PO(IComManager);
    DEFAULT_CM_PO(IComManager);
    DEFAULT_DESTRUCT_VI(IComManager);
public:
    using ComFactoryFunc = ResultCode(*)(const UUID& iid, void** pInterface, const BaseConstructionInfo* const pConstructionInfo);
    using FactoryMap = ::std::unordered_map<UUID, ComFactoryFunc>;

    struct ConstructionInfo final : BaseConstructionInfo
    {
        DEFAULT_CONSTRUCT_PU(ConstructionInfo);
        DEFAULT_CM_PU(ConstructionInfo);
        DEFAULT_DESTRUCT(ConstructionInfo);
    public:
        FactoryMap Factories;
    };
public:
    virtual ResultCode RegisterIidFactory(const UUID& iid, const ComFactoryFunc factory) noexcept = 0;
    virtual ResultCode CreateObject(const UUID& iid, void** const pInterface, const BaseConstructionInfo* const pConstructionInfo) noexcept = 0;

    template<typename T>
    ResultCode CreateObject(T** const pInterface, const typename T::ConstructionInfo* const pConstructionInfo) noexcept
    {
        return CreateObject(iid_of<T>, reinterpret_cast<void**>(pInterface), static_cast<const BaseConstructionInfo*>(pConstructionInfo));
    }

    template<typename T>
    ResultCode CreateObject(T** const pInterface) noexcept
    {
        return CreateObject(iid_of<T>, reinterpret_cast<void**>(pInterface), nullptr);
    }
};

TAU_DECL_UUID(IComManager, 0xA84460A844FB841Cull, 0x8441F8C9B9F14C8Dull);

}

extern "C" TAU_COM_LIB ::tau::com::ResultCode TauComGetComManager(::tau::com::IComManager** const pInterface) noexcept;