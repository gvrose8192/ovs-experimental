#ifndef OVS_MM_H
#define OVS_MM_H

#include <linux/overflow.h>

#ifndef HAVE_KVMALLOC_ARRAY
#ifndef HAVE_KVMALLOC_NODE
extern void *vmalloc_node(unsigned long size, int node);
#define kvmalloc_node(a, b, c) vmalloc_node(a, c)
#else
extern void *kvmalloc_node(size_t size, gfp_t flags, int node);
#endif /* HAVE_KVMALLOC_NODE */

/* RHEL 7.0 and 7.1 need this include for use of __GFP_ZERO below */
#ifndef __GFP_ZERO
#include <linux/gfp.h>
#endif

static inline void *kvmalloc(size_t size, gfp_t flags)
{
	return kvmalloc_node(size, flags, NUMA_NO_NODE);
}
static inline void *kvzalloc_node(size_t size, gfp_t flags, int node)
{
	return kvmalloc_node(size, flags | __GFP_ZERO, node);
}
static inline void *kvzalloc(size_t size, gfp_t flags)
{
	return kvmalloc(size, flags | __GFP_ZERO);
}

static inline void *kvmalloc_array(size_t n, size_t size, gfp_t flags)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;

	return kvmalloc(bytes, flags);
}

static inline void *kvcalloc(size_t n, size_t size, gfp_t flags)
{
	return kvmalloc_array(n, size, flags | __GFP_ZERO);
}

#endif
#include_next <linux/mm.h>
/*
 * Under normal circumstances the proper way to fix this up permanently
 * would be to pull in some new headers and C modules.  It's a huge overkill
 * for a few lines of code needed to fix up builds for RHEL 7.0 and 7.1
 * Recently a new requirement was added to support these releases for
 * internal builds only.
 * https://jira.eng.vmware.com/browse/CPDP-6122
 */
#if RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(7,2)
#ifndef GENMASK
#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#endif
#include <linux/slab.h>
static inline void rpl_kvfree(const void *addr)
{
	if (is_vmalloc_addr(addr))
		vfree(addr);
	else
		kfree(addr);
}
#define kvfree rpl_kvfree
#endif

#endif /* OVS_MM_H */

