WIP
# Libbloomfilter - A lock-less bloom filter implemented in c

Uses xxh3 for hashing
Uses 2 keys on compile time sized defined table

## Usage
### Normal
```c
```
### Swap

#### Construction
``bloomfilterswap_new(allocator)`` Create a new swaping bloomfilter

``bloomfilterswap_destroy(filter, deallocator)`` Destroy the filter
#### Operations
``bloomfilterswap_swap(filter)``  Changes active filter, and clears the background filter

``bloomfilterswap_add(filter, key, keylen)`` adds key to background and active filter

``bloomfilterswap_test(filter, key, keylen)`` Checks active filter for key

#### LUA
```openresty
worker_processes  12;

events{}

http {
    init_by_lua_block {
        local err
        bloom, err = require("bloomfilter")()
        if not bloom then
            return ngx.log(ngx.ERR, "INIT_ERR: ", err)
        end
        _G.bloom = bloom
    }

    server {
        listen 8080;
        location / {
            content_by_lua_block {
                if ngx.var.uri == "/swap" then
                    bloom.swap()
                    return ngx.say("SWAP");
                end
                if ngx.var.request_method == "POST" then
                    bloom.add(ngx.var.uri);
                    return ngx.say("OK")
                end
                ngx.log(ngx.ERR, bloom.test(ngx.var.uri));
                return bloom.test(ngx.var.uri) and ngx.say("HIT") or ngx.exit(404)
            }
        }
    }
}
```
