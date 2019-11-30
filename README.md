# Tachyon - C++ library of concurrency primitives

The following synchronization primitives will be supported:
- [ ] Spinlocks with different locking schemes:
    * [X] ```spinlock_cas``` - Basic unfair spinlock utilizing atomic ```compare-and-swap``` operation
    * [X] ```spinlock_tas``` - Basic unfair spinlock utilizing atomic ```test-and-set``` operation
    * [ ] ```spinlock_anderson``` - Scalable, fast and fair spinlock based on Anderson's array-based queuing lock
    * [ ] ```spinlock_clh``` - An efficient implementation of the scalable CLH lock, providing many of the same performance properties of MCS with a better fast-path.
    * [ ] ```spinlock_HCLH``` - A NUMA-friendly CLH lock
    * [ ] ```spinlock_mcs``` - An implementation of the seminal scalable and fair MCS lock
    * [ ] ```spinlock_ticket``` - An implementation of fair centralized locks
    * [ ] ...
- [ ] Seqlock (sequential lock)
- [ ] RCU (```read-copy-update```)
- [ ] ...

The following concurrency patterns will be supported:
- [ ] Disruptor
- [ ] ...


