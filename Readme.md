# Purpose
Generate bulk mongo db ids, hash them and generate qrcode pngs base 64 strings

# dependencies
libpng-dev
libqrencode-dev

# TODO
Randomness of mongo ids, since this is multithreaded, good chances of dublicate ids. For now, we can create index in DB which we will save generated ids