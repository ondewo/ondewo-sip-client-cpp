// Assertions against the concrete C++ types the SIP stubs generate.
//
// This is the per-product half of the suite: it names ondewo::sip types, so replicating
// the suite to another ONDEWO client means rewriting this file against that product's
// messages and services. Everything generic lives in test_generated_stubs.cc.
//
// Two of the shapes the NLU suite carries have no counterpart here and are deliberately
// absent rather than invented:
//   - ondewo/sip/sip.proto declares no proto3 `optional` field, so there is no explicit-
//     presence pair to assert (`grep -n "^[[:space:]]*optional " ondewo-sip-api/**/*.proto`
//     comes back empty). The plain-scalar half of that pair is kept below: it is the wire
//     contract that holds for every proto3 field SIP does declare.
//   - the Sip service declares no streaming RPC at all (`grep -n "rpc .*stream"` is empty),
//     so no ClientReader / ClientWriter / ClientReaderWriter type is generated to drive.

#include <chrono>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include <google/protobuf/empty.pb.h>

#include "ondewo/sip/sip.grpc.pb.h"
#include "ondewo/sip/sip.pb.h"

namespace ondewo_client_test {
namespace {

// A channel to a port nothing listens on. gRPC connects lazily, so constructing stubs
// against it touches no network at all; the one test that does issue an RPC gives it a
// short deadline and asserts only that the call comes back as a failure.
std::shared_ptr<grpc::Channel> DeadChannel() {
  return grpc::CreateChannel("127.0.0.1:1", grpc::InsecureChannelCredentials());
}

TEST(TypedApi, MessageSurvivesSerializeAndParse) {
  ondewo::sip::SipStatus original;
  original.set_account_name("sip:agent@ondewo.com");
  original.set_status_type(ondewo::sip::SipStatus::OUTGOING_CALL_CONNECTED);
  original.set_callee_id("sip:callee@ondewo.com");
  original.set_nlu_session_name("projects/p/agent/sessions/s");
  (*original.mutable_headers())["X-Ondewo-Call-Id"] = "call-42";

  std::string bytes;
  ASSERT_TRUE(original.SerializeToString(&bytes));
  EXPECT_FALSE(bytes.empty());

  ondewo::sip::SipStatus parsed;
  ASSERT_TRUE(parsed.ParseFromString(bytes));

  EXPECT_EQ(parsed.account_name(), "sip:agent@ondewo.com");
  EXPECT_EQ(parsed.status_type(), ondewo::sip::SipStatus::OUTGOING_CALL_CONNECTED);
  EXPECT_EQ(parsed.callee_id(), "sip:callee@ondewo.com");
  EXPECT_EQ(parsed.nlu_session_name(), "projects/p/agent/sessions/s");
  ASSERT_EQ(parsed.headers().size(), 1u);
  ASSERT_EQ(parsed.headers().count("X-Ondewo-Call-Id"), 1u);
  EXPECT_EQ(parsed.headers().at("X-Ondewo-Call-Id"), "call-42");
  EXPECT_EQ(parsed.SerializeAsString(), bytes);
}

// A plain (non-optional) proto3 scalar must keep its zero value off the wire and put a
// non-zero one on it. SIP declares no `optional` field, so this is the whole of the
// presence contract its stubs have to honour.
TEST(TypedApi, PlainScalarZeroValueStaysOffTheWire) {
  ondewo::sip::SipEndCallRequest request;
  request.set_hard_hangup(false);
  EXPECT_TRUE(request.SerializeAsString().empty());

  request.set_hard_hangup(true);
  EXPECT_FALSE(request.SerializeAsString().empty());

  ondewo::sip::SipStartSessionRequest session;
  session.set_auto_answer_interval(0);
  EXPECT_TRUE(session.SerializeAsString().empty());

  session.set_auto_answer_interval(3);
  EXPECT_FALSE(session.SerializeAsString().empty());
}

// StatusType is nested inside SipStatus, so protoc emits it as SipStatus_StatusType at
// namespace scope plus the SipStatus::StatusType alias and one constant per value.
TEST(TypedApi, EnumZeroValueIsTheUnspecifiedOne) {
  EXPECT_EQ(static_cast<int>(ondewo::sip::SipStatus::NO_SESSION), 0);
  EXPECT_EQ(ondewo::sip::SipStatus::StatusType_Name(ondewo::sip::SipStatus::NO_SESSION),
            "NO_SESSION");
  EXPECT_EQ(ondewo::sip::SipStatus_StatusType_Name(ondewo::sip::SipStatus_StatusType_NO_SESSION),
            "NO_SESSION");

  ondewo::sip::SipStatus::StatusType parsed = ondewo::sip::SipStatus::READY;
  ASSERT_TRUE(ondewo::sip::SipStatus::StatusType_Parse("NO_SESSION", &parsed));
  EXPECT_EQ(parsed, ondewo::sip::SipStatus::NO_SESSION);

  // A status defaults to the zero value, so the zero value has to be representable.
  ondewo::sip::SipStatus status;
  EXPECT_EQ(status.status_type(), ondewo::sip::SipStatus::NO_SESSION);
}

TEST(TypedApi, ServiceStubsAreConstructibleAgainstAChannel) {
  const std::shared_ptr<grpc::Channel> channel = DeadChannel();
  ASSERT_NE(channel, nullptr);

  std::unique_ptr<ondewo::sip::Sip::Stub> sip = ondewo::sip::Sip::NewStub(channel);

  EXPECT_NE(sip, nullptr);
}

TEST(TypedApi, ServicesKeepTheirFullyQualifiedNames) {
  EXPECT_STREQ(ondewo::sip::Sip::service_full_name(), "ondewo.sip.Sip");
}

// Actually issue an RPC. Nothing is listening, so the only correct outcome is a failure -
// but reaching a transport-level failure means the stub, the request/response types and
// the generated method descriptor all linked and dispatched. A crash or an OK here would
// mean the generated client is broken.
TEST(TypedApi, UnaryRpcAgainstADeadEndpointFailsCleanly) {
  std::unique_ptr<ondewo::sip::Sip::Stub> sip = ondewo::sip::Sip::NewStub(DeadChannel());

  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

  ondewo::sip::SipStartCallRequest request;
  request.set_callee_id("sip:callee@ondewo.com");
  ondewo::sip::SipStatus response;

  const grpc::Status status = sip->SipStartCall(&client_context, request, &response);

  EXPECT_FALSE(status.ok()) << "an RPC to a dead endpoint reported success";
  EXPECT_TRUE(status.error_code() == grpc::StatusCode::UNAVAILABLE ||
              status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED)
      << "unexpected status " << status.error_code() << ": " << status.error_message();
}

// Five of the eleven SIP RPCs take google.protobuf.Empty rather than a SIP message. That
// request type lives in libprotobuf, not in api/, so dispatching one proves the generated
// service links against the well-known types as well as against its own.
TEST(TypedApi, UnaryRpcWithAWellKnownRequestTypeFailsCleanly) {
  std::unique_ptr<ondewo::sip::Sip::Stub> sip = ondewo::sip::Sip::NewStub(DeadChannel());

  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

  const google::protobuf::Empty request;
  ondewo::sip::SipStatus response;

  const grpc::Status status = sip->SipGetSipStatus(&client_context, request, &response);

  EXPECT_FALSE(status.ok()) << "an RPC to a dead endpoint reported success";
  EXPECT_TRUE(status.error_code() == grpc::StatusCode::UNAVAILABLE ||
              status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED)
      << "unexpected status " << status.error_code() << ": " << status.error_message();
}

}  // namespace
}  // namespace ondewo_client_test
