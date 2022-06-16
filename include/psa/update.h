/*
 * Copyright (c) 2022 Arm Limited and Contributors.
 *
 * Candidate header for psa/update.h version 1.0
 * This proposal is manually written and provided to gather feedback.
 */

/** The major version of this implementation of the API.  */
#define PSA_FWU_API_VERSION_MAJOR 0

/** The minor version of this implementation of the API.  */
#define PSA_FWU_API_VERSION_MINOR 9

/* Use the common status definitions for all PSA APIs */
#include "psa/error.h"

/* The following definitions are all shared with other PSA APIs,
   and are or will be defined in psa/error.h */

/** Function return status.  */
typedef int32_t psa_status_t;

/** The action was completed successfully. */
#define PSA_SUCCESS ((psa_status_t)0)

// There was a communication failure inside the implementation.
#define PSA_ERROR_COMMUNICATION_FAILURE ((psa_status_t)-145)
// A tampering attempt was detected.
#define PSA_ERROR_CORRUPTION_DETECTED   ((psa_status_t)-151)
// Stored data has been corrupted.
#define PSA_ERROR_DATA_CORRUPT          ((psa_status_t)-152)
// Data read from storage is not valid for the implementation.
#define PSA_ERROR_DATA_INVALID          ((psa_status_t)-153)
// Asking for an item that doesn't exist.
#define PSA_ERROR_DOES_NOT_EXIST        ((psa_status_t)-140)
// An error occurred that does not correspond to any defined failure cause.
#define PSA_ERROR_GENERIC_ERROR         ((psa_status_t)-132)
// A hardware failure was detected.
#define PSA_ERROR_HARDWARE_FAILURE      ((psa_status_t)-147)
// Return this error when there's insufficient data when attempting to read from a resource.
#define PSA_ERROR_INSUFFICIENT_DATA     ((psa_status_t)-143)
// There is not enough runtime memory.
#define PSA_ERROR_INSUFFICIENT_MEMORY   ((psa_status_t)-141)
// There is not enough persistent storage.
#define PSA_ERROR_INSUFFICIENT_STORAGE  ((psa_status_t)-142)
// The parameters passed to the function are invalid.
#define PSA_ERROR_INVALID_ARGUMENT      ((psa_status_t)-135)
// The signature, MAC or hash is incorrect.
#define PSA_ERROR_INVALID_SIGNATURE     ((psa_status_t)-149)
// The requested action is denied by a policy.
#define PSA_ERROR_NOT_PERMITTED         ((psa_status_t)-133)
// The requested operation or a parameter is not supported by this implementation.
#define PSA_ERROR_NOT_SUPPORTED         ((psa_status_t)-134)
// There was a storage failure that might have led to data loss.
#define PSA_ERROR_STORAGE_FAILURE       ((psa_status_t)-146)


/** Status codes specific to the Firmware Update API */

/** Error codes */

// The firmware image does not apply to this device
#define PSA_ERROR_WRONG_DEVICE          ((psa_status_t)-155)
// Unsatisfied dependencies
#define PSA_ERROR_DEPENDENCY_NEEDED     ((psa_status_t)-156)
// Another installation is already running
#define PSA_ERROR_CURRENTLY_INSTALLING  ((psa_status_t)-157)
// Firmware image already installed
#define PSA_ERROR_ALREADY_INSTALLED     ((psa_status_t)-158)
// Install was previously interrupted
#define PSA_ERROR_INSTALL_INTERRUPTED   ((psa_status_t)-159)
// Flash abuse detected
#define PSA_ERROR_FLASH_ABUSE           ((psa_status_t)-160)
// There is not enough power to complete the operation
#define PSA_ERROR_INSUFFICIENT_POWER    ((psa_status_t)-161)
// Firmware decryption failure
#define PSA_ERROR_DECRYPTION_FAILURE    ((psa_status_t)-162)

// Q: should be keep missing manifest if it is optional?
#define PSA_ERROR_MISSING_MANIFEST ((psa_status_t)-163)

/** Status codes indicating success, or incomplete success */

/**
 * The action was completed successfully and requires a system reboot to complete
 * installation.
 */
#define PSA_SUCCESS_REBOOT ((psa_status_t)+1)

/**
 * The action was completed successfully and requires a restart of the component
 * to complete installation.
 */
#define PSA_SUCCESS_RESTART ((psa_status_t)+2)

/**
 * The requested operation is still running. The caller is expected to keep calling
 * the function at regular intervals until the operation completes with either
 * success or failure, and to monitor the call for timeouts.
 */
#define PSA_OPERATION_STILL_RUNNING ((psa_status_t)+4)


/**
 * Firmware Component type
 * 8 bits seems to be enough to differentiate firmware component types.
 * These are allocated by the implementation or integration.
 */
typedef uint8_t psa_fwu_component_t;

/** Version information about a firmware image */
typedef struct psa_fwu_image_version_t {
    uint8_t  major;
    uint8_t  minor;
    uint16_t patch;
    uint32_t build;
} psa_fwu_image_version_t;


/*
 * Firmware store
 * One function to request information about the storage space for a given
 * firmware component.
 */

/*
 * Size of the vendor-reserved area for static information about a firmware
 * storage area. The value is Implementation Defined.
 */
#define PSA_FWU_STORE_INFO_SZ   /* implementation-specific value */

/** Information about a firmware store */
typedef struct {
    /* State of Firmware store */
    uint8_t state;
    /* Error for Second image when store state is REJECTED or FAILED */
    psa_status_t error;
    /* Version of Active image */
    psa_fwu_image_version_t version;

    uint32_t max_size;   // Maximum image size in bytes
    uint32_t flags;      // Storage flags
    uint32_t location;   // Implementation-defined image location

    /*
     * These bytes are reserved for generic vendor usage,
     * e.g. store information about encryption, compression.
     */
    uint8_t  vendor[PSA_FWU_STORE_INFO_SZ] ;
} psa_fwu_store_info_t ;

/**
 * Returns all information for a FW image store.
 *
 * @param component Firmware component for which live store information is requested
 * @param info      Output parameter for image information related to the image ID.
 *
 */
psa_status_t psa_fwu_query(psa_fwu_component_t component,
                           psa_fwu_store_state_t *info);


/*
 * State values for a Firmware store
 */

#define PSA_FWU_READY       0x00
#define PSA_FWU_WRITING     0x01
#define PSA_FWU_STAGED      0x02
#define PSA_FWU_FAILED      0x03
#define PSA_FWU_TRIAL       0x04
#define PSA_FWU_REJECTED    0x05
#define PSA_FWU_UPDATED     0x06
#define PSA_FWU_CLEANING    0x07


/**
 * The maximum permitted size for data passed to psa_fwu_write(), in bytes.
 * The specific value is Implementation-defined and must be greater than 0.
 */
#define PSA_FWU_MAX_BLOCK_SIZE /* implementation-specific value */


/**
 * Start the installation of a firmware image.
 *
 * @param component   The firmware component being updated
 * @param manifest    Manifest (optional). Set to NULL if not present.
 * @param manifest_sz Manifest size (optional). Set to 0 if not present
 *
 */
psa_status_t psa_fwu_start(psa_fwu_component_t component,
                           void *manifest,
                           size_t manifest_sz);

/**
 * Write part of a firmware image to its staging area.
 *
 * @param component         The firmware component being written
 * @param image_offset      The offset of the block in the whole image. The offset of the first block is '0'.
 * @param block             A buffer containing a block of image data. This might be a complete image or a subset.
 * @param block_size        Size of ``block`` in bytes. The size must not be greater than `PSA_FWU_MAX_BLOCK_SIZE`.
 *
 */
psa_status_t psa_fwu_write(psa_fwu_component_t component,
                           size_t image_offset,
                           const void *block,
                           size_t block_size);

/**
 * Mark a staged firmware image as ready for installation
 */
psa_status_t psa_fwu_install(psa_fwu_component_t component);

/**
 * Request the reboot of the system, in order to complete firmware installation.
 */
psa_status_t psa_fwu_request_reboot();

/**
 * Request the platform to reject the update and roll back the firmware
 * component and any other image that is dependent on that firmware. This is
 * only used when the caller detects a fatal error after an update.
 *
 * @param component The firmware component being rejected and rolled back
 * @param error     An application-specific error code chosen by the application.
 *
 * If a specific error does not need to be reported, the value should be 0.
 * On success, this error is recorded in the ``error`` field of the
 * `psa_fwu_store_state_t` structure corresponding to this image.
 *
 */
psa_status_t psa_fwu_reject(psa_fwu_component_t component,
                            psa_status_t error);

/**
 * Indicate to the Update Service that the upgrade was successful. This
 * changes the image state of a firmware image and its dependencies, from
 * PSA_FWU_TRIAL to PSA_FWU_UPDATED.
 *
 * @param component     The firmware component being accepted.
 *
 */
psa_status_t psa_fwu_accept(psa_fwu_component_t component);

/**
 * Cleans the Second image area in the firmware store, aborting any ongoing
 * operation if needed.
 *
 * @param component Firmware component for which the Second area is to be cleared.
 *
 */
psa_status_t psa_fwu_clean(psa_fwu_component_t component);

/**
 * Does a single step to clean the Second image area in the firmware store. This
 * will abort an update process for components in WRITING or STAGED state.
 *
 * This function can return with status PSA_OPERATION_STILL_RUNNING before the clean
 * operation is complete. The function must be called repeatedly until it returns a
 * status that is not PSA_OPERATION_STILL_RUNNING.
 *
 * @param component Firmware component for which the Second area is to be cleared.
 *
 */
psa_status_t psa_fwu_clean_async(psa_fwu_component_t component);
