# Purpose
Generate bulk mongo db ids, hash them and generate qrcode pngs base 64 strings

# dependencies
libpng-dev
libqrencode-dev

# references
Napi reference: https://codemerx.com/blog/asynchronous-c-addon-for-node-js-with-n-api-and-node-addon-api/
qrcode logic: https://github.com/netoxygen/node-qrcodeine
mongo id generation: https://gist.github.com/solenoid/1372386

# apt commands for dependencies
apt-get install -y libpng-dev
apt-get install -y libqrencode-dev

# TODO
Randomness of mongo ids, since this is multithreaded, good chances of dublicate ids. For now, we can create index in DB which we will save generated ids