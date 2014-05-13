Drupal.behaviors.skedBehavior = {
  attach: function (context, settings) {
    // I have added Drupal.settings.sked.arg0, arg1, arg2 in sked_init.
    jQuery('#edit-jump-submit').hide();
    jQuery('#edit-jump-month').change(function() {
      window.location = '/calendar-work-date-ts/' + Drupal.settings.sked.arg1 + '/' + jQuery(this).val();
    });
  }
};

/*jQuery(document).ready(function() {






  jQuery('div.rotation').hover(function(e) {
    //console.log(jQuery(this).text());
  });

  jQuery('span.rad').hover(function(e) {
   // console.log(jQuery(this).text());
  });

  jQuery('span.code').hover(function(e) {
   // console.log(jQuery(this).text());
  });

  // Hide overlay initially.
  jQuery('#doc_overlay').hide();


  jQuery.bt.options.textzIndex = 99999;
  jQuery.bt.options.boxzIndex = 99998;
  jQuery.bt.options.wrapperzIndex = 99997;



  jQuery('span.info a').click(function(e) {
	//overlay('show', jQuery(this));
    //e.preventDefault();
  });

  jQuery('#close_overlay').click(function(e) {
    //overlay('hide');
	//e.preventDefault();
  });

*/

  /**
   * Show or hide overlay.
   * @param
   *   status = show || hide
   * @param 
   *   obj = jQuery(this)
   */
/*
  function overlay(status, obj) {
    if (status == 'show') {
	  jQuery('#doc_info').text(obj.attr('title'));
	  jQuery('#doc_overlay').show();
	  return;
    }
    else { 
	  jQuery('#doc_overlay').hide();
	  return;
    }
  }

});
*/