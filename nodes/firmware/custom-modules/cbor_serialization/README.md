```
[
 01, # version
 01, # message type
### record header ###
01, # record type
01, 02, 03, 04, # writer ID
00, 00, 00, 00, 00, 00, 00, 01, # sequence number
  ## HLC ##
  00, 00, 12, 34, # physical timestamp
  00, 00, 00, 0A, # logical timestamp

### gate report ###
01, # gate state

### gate observation ###
01, 02, 03, 05, # gate ID
01, # gate state

### gate command ###
01, 02, 03, 05, # gate ID
01, # gate state

### gate job ###
01, 02, 03, 05, # gate ID
01, 02, 03, 04 # mate ID
01, # gate state

### signature ###
bstr()
]
```
