# Experimental card-specific reset candidates — do not use

These candidates were not used to reset the board and the destructive image
has not been validated on hardware. Do not run it, especially after the board
is reissued. The board was instead reset using explicitly scoped `picotool`
operations and then received reviewed issuance firmware A.

The non-mutating RAM partition probe produced a BOOTSEL reconnect anomaly.
This branch is retained only as a diagnostic record, not as a recovery release.
The original firmware's power-loss exposure remains unresolved.
