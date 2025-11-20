#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

/**
 * @brief Type representing a store service iterator
 */
typedef max_align_t store_service_iterator_t;

/**
 * @brief Descriptor of a store advanced query.
 */
typedef struct {
    /** Key pattern to match */
    const uint8_t *key;

    /** Length of the key to match, as well as the key_mask (if not NULL). */
    size_t key_len;

    /** Mask to match the key pattern bit-wise. 0s are ignored, and 1s are matched.
     * If NULL, the key should match exactly. If not NULL, the length of key_mask must
     * match the length of key, and should be key_len.*/
    const uint8_t *key_mask;
} store_service_query_t;

/**
 * @brief Function type for storing data in the underlying storage
 *
 * @param context   Context pointer provided during service initialization
 * @param key       Pointer to the data key
 * @param key_len   Length of @p key_len key in bytes
 * @param data      Pointer to the data to setore
 * @param data_len  Length of @p data in bytes
 *
 * @retval 0 on success
 * @retval negative value on error
 */
typedef int (*store_service_put_t)(const void *context, const uint8_t *key,
                                   size_t key_len, const void *data, size_t data_len);

/**
 * @brief Function type for retrieving data from the underlying storage
 *
 * @param context   Context pointer provided during service initialization
 * @param key       Pointer to the data key
 * @param key_len   Length of @p key in bytes
 * @param data      Pointer to the buffer to store the record data
 * @param data_len  Length of @p data in bytes
 *
 * @retval 0 on success
 * @retval negative value on error
 */
typedef int (*store_service_get_t)(const void *context, const uint8_t *key,
                                   size_t key_len, void *data, size_t data_len);

/**
 * @brief Function type for deleting data from the underlying storage
 *
 * @param context   Context pointer provided during service initialization
 * @param key       Pointer to the data key
 *
 * @retval 0 on success
 * @retval negative value on error
 */
typedef int (*store_service_delete_t)(const void *context, const uint8_t *key,
                                      size_t key_len);

/**
 * @brief Function type for getting the size of the iterator
 *
 * @param context   Context pointer provided during service initialization
 *
 * @return Size of the iterator in bytes
 */
typedef size_t (*store_iterator_size_t)(const void *context);

/**
 * @brief Function type for initializing an iterator
 *
 * @param context   Pointer to the context provided during service initialization
 * @param iterator  Pointer to the iterator to initialize
 * @param query     Pointer to the query to execute in the store
 *
 * @retval 0 on success
 * @retval negative value on error
 */
typedef int (*store_iterator_init_t)(const void *context,
                                     store_service_iterator_t *iterator,
                                     const store_service_query_t *query);

/**
 * @brief Function type for iterating through an intialized iterator
 *
 * @param context       Pointer to the context provided during service initialization
 * @param iterator      Pointer to the iterator
 * @param key           Buffer to store the key
 * @param key_len       Length of @p key
 * @param data          Buffer to store the data
 * @param data_len      Pointer pointing to the length of @p key
 *
 * @retval 0 when a new value is returned
 * @retval negative value when the iterator is exhausted
 */
typedef int (*store_iterator_next_t)(const void *context,
                                     store_service_iterator_t *iterator, uint8_t *key,
                                     size_t *key_len, void *data, size_t *data_len);

/**
 * @brief Interface to the store service
 */
typedef struct {
    /** Function for storing data */
    store_service_put_t put;

    /** Function for retrieving data */
    store_service_get_t get;

    /** Function for deleting data */
    store_service_delete_t delete;

    /** Function for getting the size of the iterator */
    store_iterator_size_t iterator_size;

    /** Function for initializing an iterator */
    store_iterator_init_t iterator_init;

    /** Function to iterate through the iterator */
    store_iterator_next_t iterator_next;

} store_service_interface_t;

/**
 * @brief Store service
 */
typedef struct {
    /** Interface to the store service */
    store_service_interface_t interface;

    /** Context for the store service */
    const void *context;
} store_service_t;

/**
 * @brief Helper macro to declare and initialize a store iterator on the stack, getting
 *        the size in runtime.
 *
 * This macro will create:
 *
 * -  a variable of type size_t with the size of the buffer needed for the service
 *   specific iterator implementation. The name is the concatenation of @p name and
 *   "_needed_size".
 *
 *  - a variable of type unsigned char which is the buffer. The name is the concatenation
 *   of @p name and "_buf"
 *
 * - a variable that is the pointer to the iterator. The name is @p name.
 *
 * @param name      Name of the iterator variable
 * @param service   Pointer to the store service
 */
#define STORE_ITERATOR(name, service)                                                       \
        size_t name##_needed_size = (service)->interface.iterator_size((service)->context); \
        size_t name##_count = (name##_needed_size + sizeof(store_service_iterator_t) - 1) / \
                              sizeof(store_service_iterator_t);                             \
        store_service_iterator_t name##_buf[name##_count];                                  \
        store_service_iterator_t *name = name ##_buf

/**
 * @brief Get raw data from the store.
 *
 * @param service       Pointer to the service
 * @param key           Pointer to the key to get
 * @param key_len       Length of @p key
 * @param data          Pointer to copy the data
 * @param data_len      Length of @p data
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int store_service_get(store_service_t *service, const uint8_t *key, size_t key_len,
                      void *data, size_t data_len);

/**
 * @brief Put raw data in the store.
 *
 * @param service       Pointer to the service
 * @param key           Pointer to the key to assign to the data
 * @param key_len       Length of @p key_len
 * @param data          Pointer to the data to put
 * @param data_len      Length of @p data
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int store_service_put(store_service_t *service, const uint8_t *key, size_t key_len,
                      const void *data, size_t data_len);

/**
 * @brief Delete data from the store.
 *
 * @param service   Pointer to the service
 * @param key       Pointer to the key to get
 * @param key_len   Length of @p key
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int store_service_delete(store_service_t *service, const uint8_t *key, size_t key_len);

/**
 * @brief Initialize a store iterator created with @ref STORE_ITERATOR
 *
 * @param service       Pointer to the service
 * @param iterator      Pointer to the iterator to initialize
 * @param query         Store query to execute
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int store_service_iterator_init(store_service_t *service,
                                store_service_iterator_t *iterator,
                                store_service_query_t *query);

/**
 * @brief Get the next element in the iterator iteration
 *
 * @param service       Pointer to the service
 * @param iterator      Pointer to the iterator
 * @param key           Buffer to store the key of the element
 * @param key_len       Pointer pointing to the length of @p key
 * @param data          Buffer to store the element data. If NULL, only data length is
 *                      informed by writing to @p data_len
 * @param data_len      Pointer pointing to the length of @p data
 *
 * @retval 0 on success getting the next element
 * @retval negative value when the iterator is exhausted
 */
int store_service_iterator_next(store_service_t *service,
                                store_service_iterator_t *iterator, uint8_t *key,
                                size_t *key_len, uint8_t *data, size_t *data_len);

/**
 * @brief Check whether a given key matches a store service query.
 *
 * @param key           Key to check against @p query
 * @param ley_len       Length of @p key
 * @param query         Query to check against @p key
 *
 * @retval true if the query and key match
 * @retval false if the query and key do not match
 */
bool store_service_key_matches_query(const uint8_t *key, size_t key_len,
                                     const store_service_query_t *query);
