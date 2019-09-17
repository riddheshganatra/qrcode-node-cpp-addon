console.time('Program runtime');

const fs = require('fs');

const addon = require('bindings')('addon.node');

// const buf = fs.readFileSync('test-data');
let done = 0;
console.time(`process`)
for (let i = 0; i < 6; i++) {
    addon.processData(10, "riddhesh-", "ganatra", i, "", (res) => {
        // console.log(res.length);
        // console.log(res[0].svg);
        // console.log(res[0].svg.toString('base64'));
        // fs.writeFile(`./test${done}.svg`,res[0].svg, function(err) {
        //     if(err) {
        //         return console.log(err);
        //     }

        //     console.log(`./test${done}.svg`);
        // }); 

        // console.log("done");
        done = done + 1;
        if (done == 6) {
            console.timeEnd(`process`)
        }
    });
}