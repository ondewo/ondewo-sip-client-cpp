# Release History

*****************

## Release ONDEWO SIP C++ Client 5.7.2

### New Features

* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Keycloak token provider with offline-token start-up, matching ondewo-sip-client-python 5.7.2: the client config takes a Keycloak *offline* refresh token instead of a password, and the provider then starts with a `grant_type=refresh_token` exchange and never sends a password grant. A Keycloak config needs a password OR a refresh token; when both are set the token wins, and a refused token is never answered with a password login. The token is redacted like the password and takes part (hashed) in the shared-provider registry key, so two configs of different tenants never share a provider. In the header-only `client/ondewo/client/keycloak.h` (libcurl transport in `keycloak_curl.h`), wired into `client/ondewo/client/channel.h`.

### Bug Fixes

* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Keycloak token refresh survives every failure: a failed refresh backs off with full jitter from 5 s doubling up to 300 s, the background and the per-call refresh share that backoff, and a success resets it. A token that is still valid keeps being served when its refresh fails inside the 30 s expiry leeway; only an EXPIRED token raises the Keycloak authentication error, whatever failed underneath (unreachable endpoint, timeout, a non-2xx status, a body that is not a JSON object, a missing `access_token`, a non-numeric `expires_in`). A malformed response changes no provider state. The token-endpoint timeout is 10 s, quoted error bodies are cut at 500 characters, and no credential reaches a log line or an error message.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) A quoted error body is cut at 500 characters (UTF-8 code points, never splitting a character), as python's `body[:500]`. A provider whose `token_expiration_in_s` deadline ended its refresh is still shared by the registry; only `Stop()` / `Close()` retires it (`closed()`).

### Improvements

* Tracking API Version [5.7.0](https://github.com/ondewo/ondewo-sip-api/releases/tag/5.7.0) ( [Documentation](https://ondewo.github.io/ondewo-sip-api/) )
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Tests: the cases of the python SDK's `test_keycloak_refresh_resilience.py` and `test_client_config_refresh_token.py`, ported to `tests/test_keycloak.cc`, plus this SDK's edge cases.

*****************

## Release ONDEWO SIP C++ Client 5.7.0

### Improvements

* Tracking API Version [5.7.0](https://github.com/ondewo/ondewo-sip-api/releases/tag/5.7.0) ( [Documentation](https://ondewo.github.io/ondewo-sip-api/) )
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Generated with ondewo-proto-compiler 5.17.0 (protoc 35.1). Runtime unchanged: Debian trixie protobuf 3.21.12 / grpc 1.51.1.

*****************

## Release ONDEWO SIP C++ Client 5.5.2

### Improvements

* Tracking API Version [5.5.2](https://github.com/ondewo/ondewo-sip-api/releases/tag/5.5.2) ( [Documentation](https://ondewo.github.io/ondewo-sip-api/) )

*****************

## Release ONDEWO SIP C++ Client 5.5.0

### New Features

* Tracks [ONDEWO SIP API 5.5.0](https://github.com/ondewo/ondewo-sip-api/releases/tag/5.5.0); the stubs in `api/`
  are regenerated from it. New API surface (summarised from the API release notes, purely additive and
  wire-compatible with 5.4.0):
  * Answering machine detection: status `OUTGOING_CALL_ANSWERING_MACHINE_DETECTED`,
    `AnsweringMachineDetectionResult` and `SipStatus.amd_result`, `SipEndCallRequest.end_reason` /
    `SipEndCallRequest.amd_result`, and the in-container RPC `SipReportAnsweringMachineDetected`.
  * Call identity: `SipStatus.call_id`; requests are scoped to a call with the `x-ondewo-expected-call-id` gRPC
    metadatum.
  * `SipSetCallMediaControl`: call-scoped operator / participant media control (`MediaControlSetting`,
    `MediaControlOwner`), reported in `SipStatus.bot_muted` and `SipStatus.listening_paused`.
  * `SipStreamCallAudio`: bidirectional live call audio (LISTEN / TALK), counted in
    `SipStatus.call_audio_streams`. It is the first streaming RPC of the `Sip` service and is generated as a
    `grpc::ClientReaderWriter`.
  * Truthful transfers: `SipTransferCallRequest.outcome_timeout_ms`, `SipStatus.sip_response_code` and
    `SipEndCallRequest.EndCallReason.END_CALL_REASON_TRANSFERRED`.
  * `SipGetSipStatus` and `SipGetSipStatusHistory` declare `idempotency_level = NO_SIDE_EFFECTS` (the C++ channel
    helper still configures no retry policy).

### Improvements

* The test suite covers the new surface: `tests/product_config.cc` expects the three new RPCs and raises the sweep
  floors to the 5.5.0 counts (18 messages, 9 enums, 55 scalar fields); `tests/test_typed_api.cc` round-trips the
  answering machine fields, dispatches `SipSetCallMediaControl` with its call-scope metadata and drives the
  `SipStreamCallAudio` stream against a dead endpoint.
* The pinned `ondewo-proto-compiler` submodule moves from 5.15.2 to 5.15.5 (its fixes concern the Python, Rust and
  Node.js images; the C++ generation is unchanged).
* Tracking API Version [5.5.0](https://github.com/ondewo/ondewo-sip-api/releases/tag/5.5.0) ( [Documentation](https://ondewo.github.io/ondewo-sip-api/) )

*****************

## Release ONDEWO SIP C++ Client 5.4.1

### New Features

* TLS / mutual TLS: new header-only channel helper `ondewo/client/channel.h`, installed beside the generated
  headers and on the exported `ondewo::ondewo_sip_client` target's include path. `ondewo::client::ClientConfig`
  (`host`, `port`, `grpc_cert`, `grpc_client_cert`, `grpc_client_key`, `use_secure_channel`) takes PEM **content**,
  not file paths. `CreateChannel`, `CreateChannelCredentials`, `ValidateConfig` and `DefaultChannelArguments()`
  report errors as `grpc::Status` `INVALID_ARGUMENT`, never as exceptions.
* An empty `grpc_cert` uses the platform trust store. Half a client identity (certificate without key or key
  without certificate) and a plaintext channel with a client identity are refused before gRPC is called. A
  plaintext channel logs a warning on `std::clog` naming `host:port`. No error message renders a PEM, a key or the
  config; `ToString()` / `operator<<` print the client key as `***REDACTED***`. Bare IPv6 hosts are bracketed;
  CRLF PEMs work.
* Default channel arguments match the ONDEWO Python SDKs: keepalive every 30 s during calls with a 20 s timeout,
  `grpc.http2.max_pings_without_data=2`, reconnect backoff capped at 5 s, 2^31-1 byte send/receive message limit.
  Documented gaps: gRPC 1.51 ignores `grpc.http2.ping_timeout_ms`, and no per-method retry policy is configured.
* README: new section "TLS, mutual TLS and certificates" (modes, loading a PEM from a file, test PKI, security
  notes, troubleshooting).

### Improvements

* `tests/test_tls.cc`: real handshakes against in-process servers with a PKI generated by the openssl CLI at test
  time - TLS, mutual TLS, server rejecting a missing or foreign client certificate, wrong CA, system trust store,
  non-PEM CA (error, not crash), CRLF PEMs, server-name override, `[::1]` - plus the refusal, warning, redaction
  and default-argument cases. `make coverage` now also holds `client/` to 100 % line coverage.
* `tests/test_release_notes.sh` (CTest) pins the release-notes slice, the heading spelling, the `*****` separators
  and non-empty notes for the current version.
* The pinned `ondewo-proto-compiler` submodule moves from 5.15.1 to 5.15.2 (its fixes concern the PHP, Rust and
  JavaScript images and the release automation; the C++ generation is unchanged).
* Tracking API Version [5.4.0](https://github.com/ondewo/ondewo-sip-api/releases/tag/5.4.0) ( [Documentation](https://ondewo.github.io/ondewo-sip-api/) )

*****************

## Release ONDEWO SIP C++ Client 5.4.0

### New Features

* Initial release of `ondewo-sip-client-cpp`, the C++ gRPC client library for the
  [ONDEWO SIP API](https://github.com/ondewo/ondewo-sip-api) (SIP Telephony). The stubs are generated
  from the `ondewo-sip-api` submodule by the `ondewo-cpp-proto-compiler` image of
  [ondewo-proto-compiler](https://github.com/ondewo/ondewo-proto-compiler) 5.15.1, using protoc's
  built-in `--cpp_out` for the messages and `grpc_cpp_plugin` for the service stubs.
* Ships as a consumable CMake package: `find_package(ondewo_sip_client CONFIG REQUIRED)` plus
  `target_link_libraries(my_app PRIVATE ondewo::ondewo_sip_client)`. The package installs the generated
  headers under `include/ondewo_sip_client/`, the static archive
  `lib/libondewo_sip_client.a` and `lib/cmake/ondewo_sip_client/`, and re-finds `Protobuf` and
  `gRPC` on the consumer's machine through `find_dependency`.
* `public-api.h` is an umbrella header that includes every generated `*.pb.h` and `*.grpc.pb.h`, so a single
  `#include "public-api.h"` gives access to the whole API surface.
* The generated stub sources are committed to the repository. C++ has no package registry, so a git tag is the
  distribution channel: a plain `git clone` or a CMake `FetchContent_Declare` yields a buildable project with
  no Docker and no code generation step.
* `make build` reproduces the whole pipeline - submodule checkout at the pinned tags, compiler image build,
  code generation and a host-native CMake build - and `make test` verifies that every `.proto` produced a
  header and that the installed CMake package is consumable by a downstream project.

*****************
