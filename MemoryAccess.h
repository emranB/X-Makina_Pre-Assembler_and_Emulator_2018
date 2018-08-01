#pragma once


/* Access Cache Memory */
void AccessPrimaryMem(unsigned short, unsigned short*, unsigned int, unsigned int);

/* Access Primary Memory */
void AccessCacheMem(unsigned short, unsigned short*, unsigned int, unsigned int);

/* Update contents of Cache Memory */
void UpdateCache(unsigned short, unsigned short*, unsigned int, unsigned int, signed int, unsigned int);

/* Read from or Write to Cache */
void RW_CacheMem(unsigned short, unsigned short*, unsigned int, unsigned int, signed int);

/* Retain Cache Consistency */
void RetainCacheConsistency(unsigned short, unsigned short*, unsigned int, signed int);