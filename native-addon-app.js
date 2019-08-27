console.time('Program runtime');

const fs = require('fs');

const addon = require('bindings')('addon.node');

// const buf = fs.readFileSync('test-data');
let done = 0;
console.time(`process`)
for (let i = 0; i < 5; i++) {
    addon.processData(10000, (res) => {
        // console.log(res[0]);
        fs.writeFile(`./test${done}.svg`,res[0].svg, function(err) {
            if(err) {
                return console.log(err);
            }
        
            // console.log("The file was saved!");
        }); 
        
        console.log("done");
        done = done + 1;
        if (done == 5) {
            console.timeEnd(`process`)
        }
    });
}