console.time('Program runtime');

const fs = require('fs');

const addon = require('bindings')('addon.node');

// const buf = fs.readFileSync('test-data');
let done = 0;
console.time(`process`)
for (let i = 0; i < 1; i++) {
    addon.processData(1, (res) => {
        console.log(res);
        // fs.writeFile(`./test${done}.svg`,res[0].svg, function(err) {
        //     if(err) {
        //         return console.log(err);
        //     }
        
        //     console.log(`./test${done}.svg`);
        // }); 
        
        console.log("done");
        done = done + 1;
        if (done == 1) {
            console.timeEnd(`process`)
        }
    });
}