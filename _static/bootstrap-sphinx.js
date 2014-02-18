(function () {
  /**
   * Patch TOC list.
   *
   * Will mutate the underlying span to have a correct ul for nav.
   *
   * @param $span: Span containing nested UL's to mutate.
   * @param minLevel: Starting level for nested lists. (1: global, 2: local).
   */
  var patchToc = function ($ul, minLevel) {
    var findA;

    // Find all a "internal" tags, traversing recursively.
    findA = function ($elem, level) {
      var level = level || 0,
        $items = $elem.find("> li > a.internal, > ul, > li > ul");

      // Iterate everything in order.
      $items.each(function (index, item) {
        var $item = $(item),
          tag = item.tagName.toLowerCase(),
          $childrenLi = $item.children('li'),
          $parentLi = $($item.parent('li'), $item.parent().parent('li'));

        // Add dropdowns if more children and above minimum level.
        if (tag === 'ul' && level >= minLevel && $childrenLi.length > 0) {
          $parentLi
            .addClass('dropdown-submenu')
            .children('a').first().attr('tabindex', -1)

          $item.addClass('dropdown-menu');
        }

        findA($item, level + 1);
      });
    };

    findA($ul);
  };

  /**
   * Patch all tables to remove ``docutils`` class and add Bootstrap base
   * ``table`` class.
   */
  var patchTables = function () {
    $("table.docutils")
      .removeClass("docutils")
      .addClass("table")
      .attr("border", 0);
  };

  $(document).ready(function () {
    // Fix iPhone menu clicks.
    // From: https://github.com/twitter/bootstrap/issues/4550#issuecomment-8476763
    $('.dropdown-menu').on('touchstart.dropdown.data-api', function(e) {
      e.stopPropagation();
    });
    // See also...
    // // From: https://github.com/twitter/bootstrap/issues/4550#issuecomment-8879600
    // $('a.dropdown-toggle, .dropdown-menu a').on('touchstart', function(e) {
    //   e.stopPropagation();
    // });

    // Add styling, structure to TOC's.
    $(".dropdown-menu").each(function () {
      $(this).find("ul").each(function (index, item){
        var $item = $(item);
        $item.addClass('unstyled');
      });
      $(this).find("li").each(function () {
        $(this).parent().append(this);
      });
    });

    // Patch in level.
    //patchToc($("ul.globaltoc"), 1);
    patchToc($("ul.localtoc"), 2);

    // Add divider to local TOC if more children after.
    if ($("ul.localtoc > ul > li > ul li").length > 0) {
      $("ul.localtoc > ul > li > a")
        .first()
        .after('<li class="divider"></li>');
    }

    // Enable dropdown.
    $('.dropdown-toggle').dropdown();

    // Patch tables.
    patchTables();
  });
}());
