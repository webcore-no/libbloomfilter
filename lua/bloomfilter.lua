local ffi = require("ffi")

ffi.cdef[[
typedef struct bloomfilter_swap_s bloomfilter_swap_t;
typedef void *(*bloomfilter_allocator)(size_t);
typedef void (*bloomfilter_deallocator)(void *, size_t);

void *bloomfilter_shm_alloc(size_t);
void bloomfilter_shm_free(void *filter, size_t);

bloomfilter_swap_t *bloomfilterswap_new(bloomfilter_allocator allocator);

void bloomfilterswap_destroy(bloomfilter_swap_t **swap,
			     bloomfilter_deallocator deallocator);

void bloomfilterswap_swap(bloomfilter_swap_t *filter);
void bloomfilterswap_clear(bloomfilter_swap_t *filter);
void bloomfilterswap_add(bloomfilter_swap_t *filter, const void *key,
			 size_t keylen);

int bloomfilterswap_test(bloomfilter_swap_t *filter, const void *key,
			 size_t keylen);

]]

local C = ffi.load("libbloomfilter.so")

local bloomfilter_swap_t = ffi.typeof("bloomfilter_swap_t *[1]")

local string_check = function(filter, func, element)
    if type(element) ~= "string" then
        ngx.log(ngx.ERR, type(element));
        return nil, "element must be string type"
    end

    return func(filter, element, #element) == 1
end

local bloomfilter_gc = function(filter)
    return C.bloomfilterswap_destroy(filter, C.bloomfilter_shm_free)
end

return function()

    local filter = bloomfilter_swap_t()
    filter[0] =  C.bloomfilterswap_new(C.bloomfilter_shm_alloc)
    if filter[0] == nil then
        return nil, "failed to initialize filter"
    end

    --ffi.gc(filter, bloomfilter_gc)

    return {
        swap = function()
            return C.bloomfilterswap_swap(filter[0])
        end,
        clear = function()
            return C.bloomfilterswap_clear(filter[0])
        end,
        add = function(element)
            return string_check(filter[0], C.bloomfilterswap_add, element)
        end,

        test = function(element)
            return string_check(filter[0], C.bloomfilterswap_test, element)
        end
    }
end
