
const path = require('path');

let midiOuptutDir = '';
const midiFiles = [];
let bankOutput = '';
let bankInput = '';
let soundsOutput = '';
let soundsInput = [];

let findingMidi = false;
let findingSounds;

for (let i = 2; i < process.argv.length; ++i) {
    const curr = process.argv[i];
    if (curr == '-m') {
        midiOuptutDir = process.argv[i + 1];
        findingMidi = true;
        i += 1;
    } else if (findingMidi) {
        if (curr[0] == '-') {
            findingMidi = false;
        } else {
            midiFiles.push(curr);
        }
    }

    if (curr == '-i') {
        bankOutput = process.argv[i + 1];
        bankInput = process.argv[i + 2];
        i += 2;
    }

    if (curr == '-s') {
        soundsOutput = process.argv[i + 1];
        findingSounds = true;
        i += 1;
    } else if (findingSounds) {
        if (curr[0] == '-') {
            findingSounds = false;
        } else {
            soundsInput.push(curr);
        }
    }
}

console.log(`

.\\tools\\sfz2n64.exe --compress -o "${bankOutput}" "${bankInput}"
.\\tools\\sfz2n64.exe --compress -o "${soundsOutput}" ${soundsInput.map(sound => `"${sound}"`).join(' ')}
${midiFiles.map(midi => {
    return `.\\tools\\midicvt.exe ${midi} ${path.join(midiOuptutDir, path.basename(midi))}`;
}).join('\n')}

.\\tools\\applypatch.exe patch

`);