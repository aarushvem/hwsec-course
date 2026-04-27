## 1-2

**In a 64-bit system using 4KB pages, which bits are used to represent the page offset, and which are used to represent the page number?**

In a 64-bit system using 4KB pages, bits 0-11 represent the page offset and bits 12-63 represent the page number.

**How about for a 64-bit system using 2MB pages? Which bits are used for page number and which are for page offset?**

In a 64-bit system using 2MB pages, bits 0-20 represent the page offset and bits 21-63 represent the page number.

**In a 2GB buffer, how many 2MB hugepages are there?**

In a 2GB buffer, there are 1024 2MB hugepages.

## 2-1

**Given a victim address 0x752C3000, what is the value of its Row id? The value of its Column id?**

**For the same address, assume an arbitrary XOR function for computing the Bank id, list all possible attacker addresses whose Row id is one more than 0x752C3000's Row id and all the other ids match, including the Bank id and Column id. Hint: there should be 16 such addresses total.**

The attacker row must have Row id = 0x3A97 (one more than victim), the same Column id = 0x1000, and the same Bank id. Since the XOR bank function is unknown, we enumerate all 16 combinations of bits 13–16, which are the free bank bits between the fixed column and row fields. The base address is (0x3A97 << 17) | 0x1000 = 0x752E1000, and the 16 candidates are:
0x752E1000, 0x752E3000, 0x752E5000, 0x752E7000, 0x752E9000, 0x752EB000, 0x752ED000, 0x752EF000, 0x754E1000, 0x754E3000, 0x754E5000, 0x754E7000, 0x754E9000, 0x754EB000, 0x754ED000, 0x754EF000

## 2-3

**Analyze the statistics produced by your code when running part2, and report a threshold to distinguish the bank conflict.**

After running part 2, the threshold to distinguish bank conflicts is around 380.

## 3-2

**Based on the XOR function you reverse-engineered, determine which of the 16 candidate addresses you derived in Discussion Question 2-1 maps to the same bank.**

Using the reverse-engineered XOR function F0 {A14^A17, A15^A18, A16^A19, A7^A8^A9^A12^A13^A15^A16}, we computed the bank ID of the victim address 0x96ec3000 and all 16 candidate attacker addresses. The attacker address 0x96EE7000 produces the same bank ID (6) as the victim, making it the address among the 16 candidates that maps to the same bank and can be used as an aggressor row in a double-sided Rowhammer attack.

## 4-2

**Try different data pattern and include the bitflip observation statistics in the table below. Then answer the following questions:**

**Do your results match your expectations? What is the best pattern to trigger flips effectively?**

| Data Pattern (Victim/Aggressor) | 0x00/0xff | 0xff/0x00 | 0x00/0x00 | 0xff/0xff |
|---|---|---|---|---|
| Number of Flips (100 trials) | 5 | 87 | 0 | 3 |

Yes, the results match expectations. The best pattern is victim=0xff, aggressor=0x00. This pattern works best because the victim row is fully charged (all 1s) while the aggressor rows are fully discharged (all 0s), creating maximum electrical contrast. This causes the greatest charge disturbance on the victim row during hammering, making it easier for charge to leak and flip bits from 1 to 0. The 0x00/0x00 pattern getting 0 flips also matches expectations. When victim and aggressor have identical data there is no electrical contrast and no disturbance occurs.

## 5-1

**Given the ECC type descriptions listed above, fill in the following table (assuming a data length of 4). For correction/detection, only answer "Yes" if it can always correct/detect (and "No" if there is ever a case where the scheme can fail to correct/detect). We've filled in the first line for you.**

| | 1-Repetition (No ECC) | 2-Repetition | 3-Repetition | Single Parity Bit | Hamming(7,4) |
|---|---|---|---|---|---|
| Code Rate (Data Bits / Total Bits) | 1.0 | 4/8 = 0.5 | 4/12 = 0.33 | 4/5 = 0.8 | 4/7 = 0.57 |
| Max Errors Can Detect | 0 | 1 | 2 | 1 | 2 |
| Max Errors Can Correct | 0 | 0 | 1 | 0 | 1 |


## 5-3

**When a single bit flip is detected, describe how Hamming(22,16) can correct this error.**
When a single bit flip is detected in Hamming(22,16), the syndrome (computed by XORing the stored parity bits P0–P4 with the regenerated parity bits from the data) gives a 5-bit value that directly encodes the position of the flipped bit as a 1-indexed position within the 22-bit codeword. To correct the error, simply flip the bit at position syndrome - 1 (converting to 0-indexed) in the encoded value. This restores the original correct codeword. If the overall parity bit P5 is the one that flipped (syndrome = 0 but overall parity is wrong), then only P5 needs to be flipped to correct it.


## 5-5

**Can the Hamming(22,16) code we implemented always protect us from rowhammer attacks? If not, describe how a clever attacker could work around this scheme.**

No, Hamming(22,16) cannot always protect against Rowhammer attacks. A clever attacker can work around it in two ways:

**1. Inducing double bit flips:** Hamming(22,16) can only correct single bit errors. It can detect double errors but cannot correct them. A sufficiently aggressive Rowhammer attack can cause two bits to flip within the same 22-bit protected word simultaneously. When this happens, the scheme detects the error but returns the corrupted data uncorrected, meaning the attacker successfully corrupted memory.

**2. Targeted hammering:** Since Rowhammer causes predictable bit flip directions (e.g., 1→0 with the right data pattern), an attacker who knows the ECC layout can craft their hammering to flip two specific bits that produce a valid-looking codeword with a different data value, bypassing detection entirely. The ECC would see no error while the data has been silently corrupted.
