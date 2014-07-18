{    
  'target_defaults': {
    'variables': {
      'linux_target': 0,
      'mac_target': 1,
    },
  },
  'targets': [
    {
      'target_name': 'dht-fetcher',
      'type': 'executable',
      'dependencies': [
        '../net/net.gyp:net',
      ],
      'target_conditions': [
        ['linux_target==1', {
          'sources': [
            'common/paths/fetcher_paths_linux.cc',
          ],
        }],
        ['mac_target==1', {
          'sources': [
            'common/paths/fetcher_paths_mac.mm',
          ],
        }],
      ],
      'sources': [
        #core
        'core/startup_task_runner.h',
        'core/startup_task_runner.cc',
        'core/core_main_loop.h',
        'core/core_main_loop.cc',
        'core/core_main_runner.h',
        'core/core_main_runner.cc',
        'core/core_main_parts.h',
        'core/core_main_parts.cc',
        'core/core_sub_thread.h',
        'core/core_sub_thread.cc',
        'core/main_function_params.h',
        'core/notification/notification_details.h',
        'core/notification/notification_observer.h',
        'core/notification/notification_registrar.h',
        'core/notification/notification_registrar.cc',
        'core/notification/notification_service.h',
        'core/notification/notification_service_impl.h',
        'core/notification/notification_service_impl.cc',
        'core/notification/notification_source.h',
        'core/thread/core_thread.h',
        'core/thread/core_thread_delegate.h',
        'core/thread/core_thread_impl.h',
        'core/thread/core_thread_impl.cc',
        'core/thread/core_thread_message_loop_proxy.h',
        'core/thread/core_thread_message_loop_proxy.cc',

        #common
        'common/fetcher_constants.h',
        'common/fetcher_constants.cc',
        'common/fetcher_switches.h',
        'common/fetcher_switches.cc',
        'common/fetcher_notification_types.h',
        'common/paths/fetcher_paths.h',
        'common/paths/fetcher_paths.cc',
        'common/paths/fetcher_paths_internal.h',

        'fetcher_main_runner.h',
        'fetcher_main_runner.cc',
        'main.cc',
      ],
    },
  ],
}