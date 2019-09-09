{
  'targets': [
    {
      'target_name': 'addon',
      'sources': [ 'Addon.cc', 'DataProcessingAsyncWorker.cc' ],
      "conditions":[
                ['OS=="mac"',{
                    'xcode_settings':{
                        'GCC_ENABLE_CPP_EXCEPTIONS':'YES'
                    }
                }]
            ],
            'cflags!': [ '-fno-exceptions' ],
'cflags_cc!': [ '-fno-exceptions' ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")", "./"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }
  ]
}