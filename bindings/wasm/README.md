# kiwi-nlp, 한국어 형태소 분석기 Kiwi의 TypeScript/JavaScript 바인딩

## Building

Additionally to the requirements of the main project, you need to install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) and [npm](https://docs.npmjs.com/downloading-and-installing-node-js-and-npm).

To build the package, simply run `./build.sh`.

This is currently only supported on Linux and macOS. You can run the build script on Windows by using [WSL](https://learn.microsoft.com/en-us/windows/wsl/install).

You can pass the `--demo` flag to build the demo in `package-demo` as well.
If you pass `--demo-dev`, a development server for the demo will be started.

Running the above command also automatically upgrades to package version if it doesn't match the version in the main project.

## Documentation

The documentation for the package can be generated by running `npm run doc` inside the `package` directory.

The main entry point for the API is `KiwiBuilder`, which is used to create instances `Kiwi`.

## Example Usage

```javascript
import { KiwiBuilder, Match } from 'kiwi-nlp';

async function example() {
    const builder = await KiwiBuilder.create('path to kiwi-wasm.wasm');

    const kiwi = await builder.build({
        modelFiles: {
            'combiningRule.txt': '/path/to/model/combiningRule.txt',
            'default.dict': '/path/to/model/default.dict',
            'extract.mdl': '/path/to/model/extract.mdl',
            'multi.dict': '/path/to/model/multi.dict',
            'sj.knlm': '/path/to/model/sj.knlm',
            'sj.morph': '/path/to/model/sj.morph',
            'skipbigram.mdl': '/path/to/model/skipbigram.mdl',
            'typo.dict': '/path/to/model/typo.dict',
        }
    });

    const tokens = kiwi.analyze('다음은 예시 텍스트입니다.', Match.allWithNormalizing);
    /* Output: {
        "score": -39.772212982177734,
        "tokens": [
            {
                "length": 2,
                "lineNumber": 0,
                "pairedToken": 4294967295,
                "position": 0,
                "score": -6.5904083251953125,
                "sentPosition": 0,
                "str": "다음",
                "subSentPosition": 0,
                "tag": "NNG",
                "typoCost": 0,
                "typoFormId": 0,
                "wordPosition": 0
            },
            {
                "length": 1,
                "lineNumber": 0,
                "pairedToken": 4294967295,
                "position": 2,
                "score": -1.844599723815918,
                "sentPosition": 0,
                "str": "은",
                "subSentPosition": 0,
                "tag": "JX",
                "typoCost": 0,
                "typoFormId": 0,
                "wordPosition": 0
            },
            ...
        ]
    } */
}
```