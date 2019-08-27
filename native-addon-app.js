console.time('Program runtime');

const fs = require('fs');

const addon = require('bindings')('addon.node');

// const buf = fs.readFileSync('test-data');
let done = 0;
console.time(`process`)
for (let i = 0; i < 1; i++) {
    addon.processData(2, (res) => {
        // console.log(res);
        fs.writeFile("./test.svg",res[0], function(err) {
            if(err) {
                return console.log(err);
            }
        
            console.log("The file was saved!");
        }); 
        
        console.log("done");
        done = done + 1;
        if (done == 1) {
            console.timeEnd(`process`)
        }
    });
}