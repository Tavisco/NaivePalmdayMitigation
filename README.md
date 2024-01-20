# Naive PalmDay Mitigation

Cosmetically changes the year displayed by the `DateTemplateToAscii()` function as a mitigation for the infamous [PalmDay](https://palmdb.net/about/palm-day) calendar limitation.

As of now, this patch only affects Palm OS >= 3.5 and < 5.

First and foremost, this is a mitigation, not a fix. It actually doesn't let Palm OS count after December 31, 2031, but using a clever trick, it shows dates after that by just adding 56 to whatever year it was previously drawing.

It also patches the `SelectDay` modal to tweak the max year that is possible to select.

This approach has a serious drawback though:

If you use the patch on a device that already has data, all the entries that already exists will be in the wrong year, so it's best used on a fresh device. Just disable this hack to see the 'normal' year. No harm at all using it.

Requires the use of a hack manager such as [X-Master](https://palmdb.net/app/x-master-hackmaster-successor).
