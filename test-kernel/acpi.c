#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <builtins.h>
#include <acpi.h>

struct rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t rev;
    uint32_t rsdt_addr;
    // ver 2.0 only
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct rsdt {
    struct sdt sdt;
    symbol ptrs_start;
} __attribute__((packed));

static bool use_xsdt;
static struct rsdt *rsdt;

/* This function should look for all the ACPI tables and index them for
   later use */
void acpi_init(struct rsdp *rsdp) {
    if (rsdp->rev >= 2 && rsdp->xsdt_addr) {
        use_xsdt = true;
        rsdt = (struct rsdt *)((uintptr_t)rsdp->xsdt_addr);
    } else {
        use_xsdt = false;
        rsdt = (struct rsdt *)((uintptr_t)rsdp->rsdt_addr);
    }
}

/* Find SDT by signature */
void *acpi_find_sdt(const char *signature, size_t index) {
    size_t cnt = 0;

    for (size_t i = 0; i < rsdt->sdt.length - sizeof(struct sdt); i++) {
        struct sdt *ptr;
        if (use_xsdt)
            ptr = (struct sdt *)(uintptr_t)((uint64_t *)rsdt->ptrs_start)[i];
        else
            ptr = (struct sdt *)(uintptr_t)((uint32_t *)rsdt->ptrs_start)[i];

        if (!strncmp(ptr->signature, signature, 4) && cnt++ == index) {
            return (void *)ptr;
        }
    }

    return NULL;
}
