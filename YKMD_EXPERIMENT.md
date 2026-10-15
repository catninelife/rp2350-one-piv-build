# RP2350 existing-key ECDH experiment

This branch builds two experimental UF2 profiles through manual GitHub Actions:

- `original-atr`: original ATR, with PIV 9D ECDH and the administration guard.
- `ykmd-atr`: the same changes plus a protocol-compatibility ATR accepted by the investigated YubiKey minidriver. USB branding remains DGSRZ DGS-Seckey; this is not genuine Yubico hardware or a vendor-supported configuration.

Pinned firmware: `polhenarejos/pico-openpgp` commit `c4869b583fae346fb1f6bd6fef7b7fb833c12c43` (v5.0). Its original PIV implementation accepts ECC generation and signatures but lacks tag-85 ECDH. The helper is adapted from upstream commit `50d024dc535ad8d21bf8561e1b12d7e52a64f47f`, `src/piv/piv.c`. Preserve the upstream license notices and source availability when distributing derivatives.

Pinned Pico Keys SDK: `b35df81a03ed4248680bc5f2099d4fd97e71baf8`. Pinned Raspberry Pi Pico SDK 2.2.0: `a1438dff1d38bd9c65dbd693f0e5db4b9ae91779`. No filesystem or key-storage migration code is changed.

`PICO_PIV_ECDH_TEST` adds ECDH to existing PIV 9D P-256/P-384 keys. `PICO_YKMD_EXISTING_KEYS_ONLY` denies PIV administration, management-key authentication, protected management objects, key generation/import/moves, reset, PIN/PUK changes, and unknown PIV commands. Existing 9A RSA operations and standard PIN verification remain available. This policy is scoped to the PIV applet; it is not a global write prohibition for every applet or internal retry accounting.

This profile cannot provision a blank card or change a forgotten PIN. Use an independently reviewed provisioning firmware for recovery. Do not factory-reset a deployed card as part of this test. Public certificate backups cannot restore hardware private keys.

The guard is compiled and exercised as native C before firmware builds. UF2, ELF, map files, applied diff, fixed submodule revisions and checksums are published as build artifacts. Build success is not hardware validation. Before flashing, independently compare UF2 write addresses and partition information against the baseline; after flashing, compare existing certificate identities and public-key metadata before entering a PIN. Validate ECDH against an independent peer, then Windows KSP, protected-process loading, logon, and EFS/card-removal behavior separately.

No minidriver binary, card certificate, card dump, user PIN or private key is included in this repository change.
