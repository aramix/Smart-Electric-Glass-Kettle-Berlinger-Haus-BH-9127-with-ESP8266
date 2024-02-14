import FS from 'fs';
import path from 'path';
import { gzipAsync } from '@gfx/zopfli';

const SAVE_PATH = '../src';

// Check if save path exists
if (!FS.existsSync(SAVE_PATH)) {
  throw new Error(`Save path ${SAVE_PATH} does not exist`);
}

const INDEX_HTML = FS.readFileSync(path.resolve(path.resolve(), './src/index.html'));
const SCRIPT_JS = FS.readFileSync(path.resolve(path.resolve(), './src/script.js'));

function chunkArray(myArray, chunk_size) {
  var index = 0;
  var arrayLength = myArray.length;
  var tempArray = [];
  for (index = 0; index < arrayLength; index += chunk_size) {
    let myChunk = myArray.slice(index, index + chunk_size);
    // Do something if you want with the group
    tempArray.push(myChunk);
  }
  return tempArray;
}

function addLineBreaks(buffer) {
  let data = '';
  let chunks = chunkArray(buffer, 30);
  chunks.forEach((chunk, index) => {
    data += chunk.join(',');
    if (index + 1 !== chunks.length) {
      data += ',\n';
    }
  });
  return data;
}

(async () => {
  try {
    const GZIPPED_INDEX = await gzipAsync(INDEX_HTML, { numiterations: 15 });
    const GZIPPED_JS = await gzipAsync(SCRIPT_JS, { numiterations: 15 });

    const HTML_HEADER_FILE = `#ifndef chaynik_webapge_h
#define chaynik_webpage_h

#include <Arduino.h>

extern const uint8_t chaynik_HTML[${GZIPPED_INDEX.length}];

#endif
`;

    const HTML_CPP_FILE = `#include "chaynik_webpage.h" 

const uint8_t chaynik_HTML[${GZIPPED_INDEX.length}] PROGMEM = { 
${addLineBreaks(GZIPPED_INDEX)}
};
`;

    const JS_HEADER_FILE = `#ifndef chaynik_js_h
#define chaynik_js_h

#include <Arduino.h>

extern const uint8_t chaynik_JS[${GZIPPED_JS.length}];

#endif
`;

    const JS_CPP_FILE = `#include "chaynik_js.h" 

const uint8_t chaynik_JS[${GZIPPED_JS.length}] PROGMEM = { 
${addLineBreaks(GZIPPED_JS)}
};
`;

    FS.writeFileSync(path.resolve(path.resolve(), SAVE_PATH + '/chaynik_webpage.h'), HTML_HEADER_FILE);
    FS.writeFileSync(path.resolve(path.resolve(), SAVE_PATH + '/chaynik_webpage.cpp'), HTML_CPP_FILE);
    console.log(
      `[COMPRESS.js] Compressed Bundle into chaynik_webpage.h header file | Total Size: ${(
        GZIPPED_INDEX.length / 1024
      ).toFixed(2)}KB`
    );

    FS.writeFileSync(path.resolve(path.resolve(), SAVE_PATH + '/chaynik_js.h'), JS_HEADER_FILE);
    FS.writeFileSync(path.resolve(path.resolve(), SAVE_PATH + '/chaynik_js.cpp'), JS_CPP_FILE);
    console.log(
      `[COMPRESS.js] Compressed Bundle into chaynik_js.h header file | Total Size: ${(GZIPPED_JS.length / 1024).toFixed(
        2
      )}KB`
    );
  } catch (err) {
    return console.error(err);
  }
})();
