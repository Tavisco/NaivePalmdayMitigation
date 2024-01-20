# Naive Palmday Mitigation

As of now, this patch only affects Palm OS >= 3.5 and < 5.


First and foremost, this is a mitigation, not a fix. It works by cosmetically changing the year 
that shows up on the DateTemplateToAscii() function.


It actually don't let Palm OS counts after PalmDay, BUT using a clever trick, it shows dates 
after that by just adding 56 to whatever year it was previously drawing.


It also patches the SelectDay modal to tweak the max year that is possible to select.

This approach has a serious drawback though:

If you use the patch on a device that already has data, all the entries that already 
exists will be in the wrong year so it's best used on a fresh device. Just disable 
this hack to see the 'normal' year. No harm at all using it.