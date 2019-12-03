set(TACHYON_CPP
    seqlock.cpp
)

add_library(tachyon SHARED
    ${TACHYON_CPP}
)