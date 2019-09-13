{
  'targets': [
    {
      'target_name': 'addon',
      'sources': [ 'addon/Addon.cc', 'addon/DataProcessingAsyncWorker.cc','dependencies/base64.cc' ],
     "libraries": ["-lqrencode", "-lpng"],
      "conditions":[
                ['OS=="mac"',{
                    'xcode_settings':{
                        'GCC_ENABLE_CPP_EXCEPTIONS':'YES'
                    }
                }]
            ],
            'cflags!': [ '-fno-exceptions' ],
'cflags_cc!': [ '-fno-exceptions' ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")", "./addon","./dependencies"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }
  ]
}