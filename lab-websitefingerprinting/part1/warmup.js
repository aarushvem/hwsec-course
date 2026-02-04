const runs = 10;

function measureOneLine() {
  const LINE_SIZE = 16; // 128/sizeof(double) Note that js treats all numbers as double
  let result = [];

  // Fill with -1 to ensure allocation
  const M = new Array(runs * LINE_SIZE).fill(-1);

  for (let i = 0; i < runs; i++) {
    const start = performance.now();
    let val = M[i * LINE_SIZE];
    const end = performance.now();

    result.push(end - start);
  }

  return result;
}

function measureNLines() {
  let result = [];
  N=10000
  // TODO: Exercise 1-1
  const LINE_SIZE = 16; // 128/sizeof(double) Note that js treats all numbers as double

    const M = new Array(N * LINE_SIZE).fill(1);

    let trialTimes = [];

    for (let t = 0; t < 10; t++) {

      let sum = 0; 

      const start = performance.now();

      for (let i = 0; i < N; i++) {
        sum += M[i * LINE_SIZE];
      }

      const end = performance.now();

      trialTimes.push(end - start);
    }
    const sorted = [...trialTimes].sort((a, b) => a - b);
    result.push((sorted[4] + sorted[5]) / 2)

    console.log("Hello from warmup.js"+result);

  return result;
}

document.getElementById(
  "exercise1-values"
).innerText = `1 Cache Line: [${measureOneLine().join(", ")}]`;

document.getElementById(
  "exercise2-values"
).innerText = `N Cache Lines: [${measureNLines().join(", ")}]`;
