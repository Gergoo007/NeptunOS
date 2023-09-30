# 2 MiB Paging
## Page directory entry struktúra
| Bit	| Magyarázat		|
|-------|-------------------|
| 0 	| Present			|
| 1 	| R/W (0 -> RO)		|
| 2		| U/S (0 -> S)		|
| 3		| Write Trough		|
| 4		| Cache disable		|
| 5		| Accessed (RO)		|
| 6		| Dirty (RO)		|
| 7		| Size (1 -> 2 MiB) |
| 8		| Globál (?)		|
| 9:10	| Nem számít		|
| 11	| HLAT restart		|
| 12	| Memória típus (PAT)|
| 13:20	| 0-nak kell lennie	|
| 21:47	| "Fizikai" cím		|
| 48:51	| Cím felső bitei (PML5)|
| 52:58 | Nem számít		|
| 59:62 | Protection key (?)|
| 63	| Execute Disable	|

# 4 KiB Paging
## Page table entry struktúra
| Bit	| Magyarázat		|
|-------|-------------------|
| 0 	| Present			|
| 1 	| R/W (0 -> RO)		|
| 2		| U/S (0 -> S)		|
| 3		| Write Trough		|
| 4		| Cache disable		|
| 5		| Accessed (RO)		|
| 6		| Dirty (RO)		|
| 7		| Size (1 -> 2 MiB) |
| 8		| Globál (?)		|
MAJD FOLYTATNI KELL
