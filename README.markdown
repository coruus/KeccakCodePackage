This branch contains an unifdeffed and (slightly) cleaned-up version of
the Keccak Team's Keccak Code Package.

It only supports little-endian platforms; it may be useful to other
implementors of Keccak on little-endian platforms, as it is somewhat
easier to read.

It is likely not useful for merging upstream; I have no intentions of
maintaining or extending this branch (though I will attempt to backport
any fixes to correctness errors accepted upstream).


The original README:

    The Keccak Code Package

    This project gathers different implementations of the Keccak sponge function family. Its purpose is to replace the implementations originally in the "Keccak Reference and Optimized Code in C" with cleaner and more flexible code following the interface proposed in the note "A software interface for Keccak".

    More information can be found:
    * on the proposed software interface at http://keccak.noekeon.org/NoteSoftwareInterface.pdf
    * on Keccak in general at http://keccak.noekeon.org/

    Code contributions are welcome.

    The Keccak Team, June 2013
    Guido Bertoni, Joan Daemen, Michaël Peeters and Gilles Van Assche

    Acknowledgments:
    - 32-bit in-place implementation by Ronny Van Keer
    - genKAT.c based on the SHA-3 contest's genKAT.c by NIST
    - brg_endian.h by Brian Gladman
