// Number of sweep counts
// TODO (Exercise 2-1): Choose an appropriate value!
let P = 5;

// Number of elements in your trace
let K = Math.floor(5000 / P);

const buffer = new Uint8Array(8 * 1024 * 1024);

for (let i = 0; i < buffer.length; i++) {
  buffer[i] = (buffer[i] + 1) & 0xff;
}

let sink = 0;

// Array of length K with your trace's values
let T;

// Value of performance.now() when you started recording your trace
let start;

const STRIDE = 64; 

function doOneSweep() {
  for (let i = 0; i < buffer.length; i += STRIDE) {
    sink ^= buffer[i];
  }  
}


function record() {
  // Create empty array for saving trace values
  T = new Array(K);
 
  // Fill array with -1 so we can be sure memory is allocated
  T.fill(-1, 0, T.length);

  // Save start timestamp`
  start = performance.now();

  // TODO (Exercise 2-1): Record data for 5 seconds and save values to T.

  

  for (let i = 0; i < K; i++) {
    const windowEnd = performance.now() + P;
    let count = 0;

    while (performance.now() < windowEnd) {
      doOneSweep();
      count++
    }
    T[i] = count;
  }


  // Once done recording, send result to main thread
  postMessage(JSON.stringify(T));
}

// DO NOT MODIFY BELOW THIS LINE -- PROVIDED BY COURSE STAFF
self.onmessage = (e) => {
  if (e.data.type === "start") {
    setTimeout(record, 0);
  }
};
