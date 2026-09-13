#include "product_config.h"

namespace ondewo_client_test {

// Mirrors the #include list of the generated public-api.h, one .proto per pair of headers:
//   sed -n 's|^#include "\(.*\)\.pb\.h"$|\1|p' public-api.h | sed 's|\.grpc$||' | sort -u
//
// The ONDEWO SIP API is a single .proto file. Its imports (google/protobuf/empty.proto,
// timestamp.proto) are well-known types that live inside libprotobuf, so no google/* code is
// generated into api/ and none is listed here.
const std::vector<std::string> kProtoFileNames = {
    "ondewo/sip/sip.proto",
};

const std::vector<std::string> kServiceFullNames = {
    "ondewo.sip.Sip",
};

const std::vector<ExpectedMethod> kExpectedMethods = {
    // The session lifecycle ...
    {"ondewo.sip.Sip", "SipStartSession"},
    {"ondewo.sip.Sip", "SipEndSession"},
    {"ondewo.sip.Sip", "SipRegisterAccount"},
    // ... the call lifecycle ...
    {"ondewo.sip.Sip", "SipStartCall"},
    {"ondewo.sip.Sip", "SipEndCall"},
    {"ondewo.sip.Sip", "SipTransferCall"},
    // ... the status RPCs, both taking google.protobuf.Empty rather than a SIP request ...
    {"ondewo.sip.Sip", "SipGetSipStatus"},
    {"ondewo.sip.Sip", "SipGetSipStatusHistory"},
    // ... and the audio controls.
    {"ondewo.sip.Sip", "SipPlayWavFiles"},
    {"ondewo.sip.Sip", "SipMute"},
    {"ondewo.sip.Sip", "SipUnMute"},
};

// SipStatus is the response type of nine of the eleven RPCs and carries eight singular
// strings plus the status enum.
const std::string kScalarMessageFullName = "ondewo.sip.SipStatus";

// SIP declares exactly one enum and it is nested inside SipStatus - a fully-qualified proto
// name separates the nesting with '.', not with C++'s '::'.
const std::string kEnumFullName = "ondewo.sip.SipStatus.StatusType";

// ONDEWO SIP API 5.4.0 generates 8 messages (the three map<> entry types excluded), 1 enum
// and 17 singular scalar fields across the file listed above. SIP is a deliberately small
// API - there is no room to round these down, so the floors ARE the current counts and any
// loss fails the sweep. A floor of 1 enum is the honest value here: sip.proto declares
// exactly one, nested inside SipStatus.
const int kMinimumMessageCount = 8;
const int kMinimumEnumCount = 1;
const int kMinimumScalarFieldCount = 17;

}  // namespace ondewo_client_test
