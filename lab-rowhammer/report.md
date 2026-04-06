## 1-2

**In a 64-bit system using 4KB pages, which bits are used to represent the page offset, and which are used to represent the page number?**

**How about for a 64-bit system using 2MB pages? Which bits are used for page number and which are for page offset?**

**In a 2GB buffer, how many 2MB hugepages are there?**

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

## 4-2

**Try different data pattern and include the bitflip observation statistics in the table below. Then answer the following questions:**

**Do your results match your expectations? What is the best pattern to trigger flips effectively?**

## 5-1

**Given the ECC type descriptions listed above, fill in the following table (assuming a data length of 4). For correction/detection, only answer "Yes" if it can always correct/detect (and "No" if there is ever a case where the scheme can fail to correct/detect). We've filled in the first line for you.**

## 5-3

**When a single bit flip is detected, describe how Hamming(22,16) can correct this error.**

## 5-5

**Can the Hamming(22,16) code we implemented always protect us from rowhammer attacks? If not, describe how a clever attacker could work around this scheme.**

