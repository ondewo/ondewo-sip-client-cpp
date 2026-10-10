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
    // ... the audio controls ...
    {"ondewo.sip.Sip", "SipPlayWavFiles"},
    {"ondewo.sip.Sip", "SipMute"},
    {"ondewo.sip.Sip", "SipUnMute"},
    // ... and the call control of API 5.5.0: the in-container answering machine report, the
    // call-scoped media control and the bidirectional live call audio stream.
    {"ondewo.sip.Sip", "SipReportAnsweringMachineDetected"},
    {"ondewo.sip.Sip", "SipSetCallMediaControl"},
    {"ondewo.sip.Sip", "SipStreamCallAudio"},
};

// SipStatus is the response type of twelve of the fourteen RPCs and carries singular
// strings, booleans and integers plus the status enum.
const std::string kScalarMessageFullName = "ondewo.sip.SipStatus";

// The call status enum is nested inside SipStatus - a fully-qualified proto
// name separates the nesting with '.', not with C++'s '::'.
const std::string kEnumFullName = "ondewo.sip.SipStatus.StatusType";

// ONDEWO SIP API 5.5.0 generates 18 messages (the map<> entry types excluded), 9 enums and
// 55 singular scalar fields across the file listed above (the sweep's own counts). SIP is a
// deliberately small API - there is no room to round these down, so the floors ARE the
// current counts and any loss fails the sweep.
const int kMinimumMessageCount = 18;
const int kMinimumEnumCount = 9;
const int kMinimumScalarFieldCount = 55;

}  // namespace ondewo_client_test
