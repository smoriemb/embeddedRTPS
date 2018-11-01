/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_TYPES_H
#define RTPS_TYPES_H

#include <lwip/ip4_addr.h>

#include <cstdint>
#include <array>
#include <initializer_list>

namespace {
    typedef uint16_t ip4_port_t;

    enum class EntityKind_t : uint8_t{
        USER_DEFINED_UNKNOWN            = 0x00,
        // No user define participant
        USER_DEFINED_WRITER_WITH_KEY    = 0x02,
        USER_DEFINED_WRITER_WITHOUT_KEY = 0x03,
        USER_DEFINED_READER_WITHOUT_KEY = 0x04,
        USER_DEFINED_READER_WITH_KEY    = 0x07,

        BUILD_IN_UNKNOWN                = 0xc0,
        BUILD_IN_PARTICIPANT            = 0xc1,
        BUILD_IN_WRITER_WITH_KEY        = 0xc2,
        BUILD_IN_WRITER_WITHOUT_KEY     = 0xc3,
        BUILD_IN_READER_WITHOUT_KEY     = 0xc4,
        BUILD_IN_READER_WITH_KEY        = 0xc7,

        VENDOR_SPEC_UNKNOWN             = 0x40,
        VENDOR_SPEC_PARTICIPANT         = 0x41,
        VENDOR_SPEC_WRITER_WITH_KEY     = 0x42,
        VENDOR_SPEC_WRITER_WITHOUT_KEY  = 0x43,
        VENDOR_SPEC_READER_WITHOUT_KEY  = 0x44,
        VENDOR_SPEC_READER_WITH_KEY     = 0x47
    };

    enum class TopicKind_t : uint8_t{
        NO_KEY   = 1,
        WITH_KEY = 2
    };

    enum class ChangeKind_t : uint8_t{
        ALIVE, NOT_ALIVE_DISPASED, NOT_ALIVE_UNREGISTERED
    };

    enum class ReliabilityKind_t : uint8_t{
        BEST_EFFORT = 1,
        RELIABLE    = 3
    };

    enum class LocatorKind_t : int32_t{
        LOCATOR_KIND_INVALID  = -1,
        LOCATOR_KIND_RESERVED =  0,
        LOCATOR_KIND_UDPv4    =  1,
        LOCATOR_KIND_UDPv6    =  2
    };

    struct GuidPrefix_t{
        std::array<uint8_t, 12> id;
    };

    struct EntityId_t{
        std::array<uint8_t, 3> entityKey;
        EntityKind_t entityKind;
    };

    struct GUID_t{
        GuidPrefix_t guidPrefix;
        EntityId_t entityId;
    };

    // Described as long but there wasn't any definition. Other than 32 bit does not conform the default values
    struct Time_t{
        int32_t seconds;           // time in seconds
        uint32_t fraction; // time in sec/2^32
    };

    struct VendorId_t{
        std::array<uint8_t, 2> vendorId;
    };

    struct SequenceNumber_t{
        int32_t high;
        uint32_t low;
    };

    struct FragmentNumber_t{
        uint32_t value;
    };

    struct Locator_t{
        static const uint32_t LOCATOR_PORT_INVALID = 0;
        static constexpr std::array<uint8_t, 16> LOCATOR_ADDRESS_INVALID = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

        LocatorKind_t kind = LocatorKind_t::LOCATOR_KIND_INVALID;
        uint32_t port = LOCATOR_PORT_INVALID;
        std::array<uint8_t,16> address = LOCATOR_ADDRESS_INVALID; // TODO make private such that kind and address always match?

        void setUDPv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
            kind = LocatorKind_t::LOCATOR_KIND_UDPv4;
            address = {0,0,0,0,0,0,0,0,0,0,0,0,a,b,c,d};
        }
    };

    struct Count_t{
        int32_t value;
    };

    struct ProtocolVersion_t{
        uint8_t major;
        uint8_t minor;
    };

    /* Default Values */
    // TODO memory_reduction_possible
    constexpr EntityId_t ENTITYID_UNKNOWN = {};
    constexpr EntityId_t ENTITYID_PARTICIPANT                            = {{00,00,01}, EntityKind_t::BUILD_IN_PARTICIPANT };
    constexpr EntityId_t ENTITYID_SEDP_BUILTIN_TOPIC_WRITER              = {{00,00,02}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
    constexpr EntityId_t ENTITYID_SEDP_BUILTIN_TOPIC_READER              = {{00,00,02}, EntityKind_t::BUILD_IN_READER_WITH_KEY};
    constexpr EntityId_t ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER       = {{00,00,03}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
    constexpr EntityId_t ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER       = {{00,00,03}, EntityKind_t::BUILD_IN_READER_WITH_KEY};
    constexpr EntityId_t ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER      = {{00,00,04}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
    constexpr EntityId_t ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER      = {{00,00,04}, EntityKind_t::BUILD_IN_READER_WITH_KEY};
    constexpr EntityId_t ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER        = {{00,01,00}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
    constexpr EntityId_t ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER        = {{00,01,00}, EntityKind_t::BUILD_IN_READER_WITH_KEY};
    constexpr EntityId_t ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER = {{00,02,00}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
    constexpr EntityId_t ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER = {{00,02,00}, EntityKind_t::BUILD_IN_READER_WITH_KEY};

    constexpr GuidPrefix_t GUIDPREFIX_UNKNOWN = {};

    constexpr Time_t TIME_ZERO = {};
    constexpr Time_t TIME_INVALID = {-1, 0xFFFFFFFF};
    constexpr Time_t TIME_INFINITE = {0x7FFFFFFF, 0xFFFFFFFF};

    constexpr VendorId_t VENDOR_UNKNOWN = {};

    constexpr SequenceNumber_t SEQUENCENUMBER_UNKNOWN = {-1, 0};

    constexpr ProtocolVersion_t PROTOCOLVERSION_1_0 = {1,0};
    constexpr ProtocolVersion_t PROTOCOLVERSION_1_1 = {1,1};
    constexpr ProtocolVersion_t PROTOCOLVERSION_2_0 = {2,0};
    constexpr ProtocolVersion_t PROTOCOLVERSION_2_1 = {2,1};
    constexpr ProtocolVersion_t PROTOCOLVERSION_2_2 = {2,2};
    constexpr ProtocolVersion_t PROTOCOLVERSION = PROTOCOLVERSION_2_2;
}

#endif //RTPS_TYPES_H