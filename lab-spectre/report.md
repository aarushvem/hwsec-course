## 1-1

**Given the attack plan above, how many addresses need to be flushed in the first step?**
256 addresses need to be flushed in the first step, since the leaked value is one byte and can take 256 possible values (0–255). Each value maps to a different page in the shared memory region, so the attacker must flush all 256 pages to ensure a clean Flush+Reload signal and accurately determine which page was accessed.

## 2-1

**Copy your code in `run_attacker` from `attacker-part1.c` to `attacker-part2.c`. Does your Flush+Reload attack from Part 1 still work? Why or why not?**

No it does not work since Part 2 added a bounds check which prevents anything other than the first 4 bytes to be loaded in non-speculatively. Any offset greater than 4 does not bring a page into cache, so Flush+Reload sees only misses

## 2-3

**In our example, the attacker tries to leak the values in the array `secret_part2`. In a real-world attack, attackers can use Spectre to leak data located in an arbitrary address in the victim's space. Explain how an attacker can achieve such leakage.**

In a real attack, the attacker can leak arbitrary memory by passing an offset that can be added to the base address of the array which is being attacked. This offset can point to any target address in the victim's space. Specifically, if the victim code does array[offset], the attacker computes offset = target_address - base_address_of_array. Since spectre is during speculative execution it can bypass the restrictions typically enforced by the OS.


## 2-4

**Experiment with how often you train the branch predictor. What is the minimum number of times you need to train the branch (i.e. `if offset < part2_limit`) to make the attack work?**

The minimum number of times to train the branch in order to consistently get accurate results was two. However, in order to ensure accuracy while keeping efficiency, we set the training rounds to 6 which consistently passes while also being quick.

## 3-2

**Describe the strategy you employed to extend the speculation window of the target branch in the victim.**



## 3-3

**Assume you are an attacker looking to exploit a new machine that has the same kernel module installed as the one we attacked in this part. What information would you need to know about this new machine to port your attack? Could it be possible to determine this infomration experimentally? Briefly describe in 5 sentences or less.**


