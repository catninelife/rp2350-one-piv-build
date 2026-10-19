# RP2350 existing-key ECDH experiment

This branch builds two experimental UF2 profiles through manual GitHub Actions:

- `original-atr`: original ATR, with PIV 9D ECDH and the administration guard.
- `ykmd-atr`: the same changes plus a protocol-compatibility ATR accepted by the investigated YubiKey minidriver. USB branding remains DGSRZ DGS-Seckey; this is not genuine Yubico hardware or a vendor-supported configuration.

Pinned firmware: `polhenarejos/pico-openpgp` commit `c4869b583fae346fb1f6bd6fef7b7fb833c12c43` (v5.0). Its original PIV implementation accepts ECC generation and signatures but lacks tag-85 ECDH. The helper is adapted from upstream commit `50d024dc535ad8d21bf8561e1b12d7e52a64f47f`, `src/piv/piv.c`. Preserve the upstream license notices and source availability when distributing derivatives.

Pinned Pico Keys SDK: `b35df81a03ed4248680bc5f2099d4fd97e71baf8`. Pinned Raspberry Pi Pico SDK 2.2.0: `a1438dff1d38bd9c65dbd693f0e5db4b9ae91779`. No filesystem or key-storage migration code is changed.

`PICO_PIV_ECDH_TEST` adds ECDH to existing PIV 9D P-256/P-384 keys. `PICO_YKMD_EXISTING_KEYS_ONLY` denies PIV administration, management-key authentication, protected management objects, key generation/import/moves, factory reset, PIN/PUK changes, and unknown PIV commands. Existing 9A RSA operations and standard PIN verification remain available. This policy is scoped to the PIV applet; it is not a global write prohibition for every applet or internal retry accounting.

The revised 2026-10-19 candidate enables `PICO_YKMD_ALLOW_PUK_RESET` and `PICO_YKMD_ALLOW_PIN_CHANGE`. It allows RESET RETRY COUNTER (`00 2C 00 80`, 8-byte PUK + 8-byte new PIN) and CHANGE REFERENCE DATA for PIN only (`00 24 00 80`, 8-byte old PIN + 8-byte new PIN). Both require exactly 16 data bytes. Unchanged upstream PIV handlers verify the credential, account for wrong attempts, reject blocked credentials and restore PIN retries on success. Successful PUK recovery is not limited to three lifetime uses; the three-attempt policy concerns incorrect credentials. PUK changes (`0x24`, P2 `0x81`), management operations and factory reset remain blocked. Both options default OFF when omitted; both profiles in this branch explicitly enable them.

This is a replacement for firmware B, the final ECDH/YKMD firmware. Firmware A, the provisioning baseline, is unchanged. A newly provisioned card still uses A then B; an already provisioned card upgrades B directly without re-provisioning through A. This profile cannot provision a blank card. PIN maintenance writes only the PIN verifier and credential retry state; no key-container or certificate write command is enabled. Blocked or unknown credentials cannot be bypassed. Do not factory-reset a deployed card. Public certificate backups cannot restore hardware private keys. This candidate has not been flashed or validated for Windows logon/EFS.

The guard is compiled and exercised as native C before firmware builds. UF2, ELF, map files, applied diff, fixed submodule revisions and checksums are published as build artifacts. Build success is not hardware validation. Before flashing, independently compare UF2 write addresses and partition information against the baseline; after flashing, compare existing certificate identities and public-key metadata before entering a PIN. Validate ECDH against an independent peer, then Windows KSP, protected-process loading, logon, and EFS/card-removal behavior separately.

No minidriver binary, card certificate, card dump, user PIN or private key is included in this repository change.
