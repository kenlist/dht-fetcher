{    
  'targets': [
    {
      'target_name': 'dht-fetcher',
      'type': 'executable',
      'dependencies': [
        '../net/net.gyp:net',
      ],
      'sources': [
        'main.cc',
      ],
    },
  ],
}