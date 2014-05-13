<?php

/**
 * Implementation of hook_views_data().
 * Let views know about xray_sked table.
 */
function sked_views_data() {

	$data = array(
		'xray_sked' => array(
			'table' => array(
							  'group' => t('X-ray'),
							  'base' => array('field' => 'sid', 'title' => t('X-ray schedule table'), 'help' => t('Table with info to tie together shift with work date and the radiologist responsible for it all.'), 'weight' => -15),
                              'join' => array('users' => array('left_field' => 'uid', 'field' => 'uid')),
                              'join' => array('node_revision' => array('left_field' => 'nid', 'field' => 'nid')),
							),
			'sid' => array(
                           'title' => t('Schedule ID'), 
                           'help' => t('Primary key'), 
							'field' => array(
							  'handler' => 'views_handler_field_numeric', 
							  'click sortable' => TRUE,
							), 
							'filter' => array(
							  'handler' => 'views_handler_filter_numeric',
							), 
							'sort' => array(
							  'handler' => 'views_handler_sort',
							),
             ),
			'uid' => array('title' => t('Drupal user id'), 'help' => t('Link schedule info with drupal user.'), 
							'field' => array(
							  'handler' => 'views_handler_field_numeric', 
							  'click sortable' => TRUE,
							), 
							'filter' => array(
							  'handler' => 'views_handler_filter_numeric',
							), 
							'sort' => array(
							  'handler' => 'views_handler_sort',
							),
							'argument' => array(
								  'handler' => 'views_handler_argument_numeric',
							),
                           'relationship' => array(
                                         'base' => 'users',
                                         'base field' => 'uid',
                                         'handler' => 'views_handler_relationship',
                                         'label' => t('User as per xray_sked'),
                                         'title' => t('xray_sked table related to users table'),
                                         'help' => t('xray_sked table has uid field so it can be related to users table'),
                           ),
               ),
			'nid' => array('title' => t('Node id'), 'help' => t('Link schedule info with node.'), // node_revision 
							'field' => array(
							  'handler' => 'views_handler_field_numeric', 
							  'click sortable' => TRUE,
							), 
							'filter' => array(
							  'handler' => 'views_handler_filter_numeric',
							), 
							'sort' => array(
							  'handler' => 'views_handler_sort',
							),
                           'relationship' => array(
                                         'base' => 'node', // node_revision
                                         'base field' => 'nid',
                                         'handler' => 'views_handler_relationship',
                                         'label' => t('Node to match up rotation info'),
                                         'title' => t('xray_sked table related to node revision table'),
                                         'help' => t('xray_sked table has nid field so it can be related to node revision table'),
                           ),
               ),
			'rad_id' => array(
							   'title' => t('Legacy radiologist ID'),
								 'help' => t('This is the ID used in the uploaded CSV file to recognise radiologist.'),
							'field' => array(
							  'handler' => 'views_handler_field_numeric', 
							  'click sortable' => TRUE,
							), 
							'filter' => array(
							  'handler' => 'views_handler_filter_numeric',
							), 
							'sort' => array(
							  'handler' => 'views_handler_sort',
							),
							'argument' => array(
								  'handler' => 'views_handler_argument_numeric',
							),
						),
			'code' => array(
							   'title' => t('Code'),
								 'help' => t('The code that represents the rotation or round.'),
								'field' => array(
								  'handler' => 'views_handler_field', 
								  'click sortable' => TRUE, // This is use by the table display plugin.
								), 
								'sort' => array(
								  'handler' => 'views_handler_sort',
								), 
								'filter' => array(
								  'handler' => 'views_handler_filter_string',
								), 
								'argument' => array(
								  'handler' => 'views_handler_argument_string',
								),
							 ),
			'code_type' => array(
							   'title' => t('Code type'),
								 'help' => t('The code type: rotation or rounds'),
								'field' => array(
								  'handler' => 'views_handler_field', 
								  'click sortable' => TRUE, // This is use by the table display plugin.
								), 
								'sort' => array(
								  'handler' => 'views_handler_sort',
								), 
								'filter' => array(
								  'handler' => 'views_handler_filter_string',
								), 
								'argument' => array(
								  'handler' => 'views_handler_argument_string',
								),
							 ),
			'call_flag' => array(
							   'title' => t('Call'),
								 'help' => t('Flag indicating whether this is a call shift or not.'),
							'field' => array(
							  'handler' => 'views_handler_field_numeric', 
							  'click sortable' => TRUE,
							), 
							'filter' => array(
							  'handler' => 'views_handler_filter_numeric',
							), 
							'sort' => array(
							  'handler' => 'views_handler_sort',
							),
							 ),
			'work_date' => array(
							   'title' => t('Work Date'),
								 'help' => t('Format in db is 2013-12-31'),
								'field' => array(
								  'handler' => 'views_handler_field', 
								  'click sortable' => TRUE, // This is use by the table display plugin.
								), 
								'sort' => array(
								  'handler' => 'views_handler_sort',
								), 
								'filter' => array(
								  'handler' => 'views_handler_filter_string',
								), 
								'argument' => array(
								  'handler' => 'views_handler_argument_string',
								),
							 ),
			'work_date_ts' => array(
							   'title' => t('Timestamp for work date.'),
							   'help' => t('This here because Drupal likes timestamps.'),
							   'field' => array(
									 'handler' => 'views_handler_field_date',
									 'click sortable' => TRUE,
								),
								'filter' => array(
									 'handler' => 'views_handler_filter_date',
								),
								'sort' => array(
									 'handler' => 'views_handler_sort_date',
								),
							 ),
		),
	);

  return $data;

}


/**
 * Implements hook_views_plugins.
 */
function sked_views_plugins() {
  $plugins = array();
  $plugins['access'] = array(
    'sked_access' => array(
      'title' => t('Sked module custom access check'),
      'handler' => 'sked_plugin_access', // _role
      'path' => drupal_get_path('module', 'sked') . '/includes/views/plugins',
      'help' => t('Sked module access plugin to provide Southlake, Mackenzie and Clinic roles with access to shift-specific schedules based on passed-in arguments. ex. Southlake role should be able to see all S1, S7 etc. shifts, but not C1, C7 etc.'),
      'uses options' => FALSE,
    ),
  );

  return $plugins;
}