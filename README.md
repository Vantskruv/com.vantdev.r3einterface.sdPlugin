# Raceroom Interface
<br>
A Streamdeck plugin for Raceroom, built to make box-this-laps easier
<br>
<br>

## What does the request box button do
<br>
Depending on the options set in this button, when pressed, you may request box this lap with prefered pit-options.
In this button we have a table of pitstop-options. You can select more than one option here. If <b>'Use only toggle buttons'</b>
is not checked, this button will use these options instead, ignoring the state of the visible toggle buttons.<br>
<br>
This button have two states. When pressed, it may take some time for it to finish, and it blinks between the
two states while executing the pit-stop actions.<br>
You can set the title and icons to your liking.
<br>
<br>
There are several options for this button.<br>
<br>
<b>* Use only toggle buttons</b><br>
When checked, this button will only used the options from the visible toggle buttons.<br>
If a toggle-button is active (green outlined), it will select its option for the pit-menu.<br>
If a toggle-button has a faint yellow outline, it deselects its option in the pit-menu if it is already active.<br>
<br>
<b>* Request box this lap when pressed</b><br>
If checked, the pit-options will be set, and if successful, it requests box this lap.<br>
Otherwise, it only changes the pit-options in the menu.<br>
<br>
<b>* Close pit menu when finished</b><br>
If pit-stop actions in the menu is set successfully, the pit-menu will be closed if this is checked.<br>
<br>

## What does the toggle button do
<br>
The purpose of the toggle button is to toggle which pit-stop action you want to do before you request box this lap.
You can only select one pit-option for this button. The option selected will be active for the pitstop-
request when the button has a green outline, and not active when the button has a faint yellow outline. Note that
the pit-options will not be applied directly ingame when the toggle-buttons is pressed. The ingame pitstop-options is set
when a request box button is pressed.
You can set the title of the toggle-button to whatever you want, preferably similar to the set option,
and you can also set the different background icons for the different states.
<br>

## The Fuel Calc option
<br>
This fuel-option calculates the required fuel for the session. It relies on your best lap-time (if running a timed session),
and Racerooms internal calculation of fuel/lap.<br>
If there is no valid lap-time (if running a timed session), or no valid fuel/lap calculation, a box-this-lap will not be
requested, and the pit-menu will not be closed to notify you about that.
It is recommened to drive a couple of clean laps with good average laptimes, to get the best approximation of how much
fuel is required til the end of the session.<br>
No responsibility from my end will be taken if you end up with not enough fuel in the end of the session.
<br>

## When it fails
<br>
If you request box this lap and some options are not available in the pit-menu, the pit-stop execution may leave
the pit-menu open and not request box this lap, even if the <b>'Close pit menu when finished is checked'</b> in the
request box button.<br>
The purpose for this is to notify the driver some important pitstop-options did not work.<br>
<br>
This will happen when:<br>
<br>
these options are active or not:<br>
* Change tires<br>
* Refuel (i.e. this can happen if the 'Calc' option is set with no valid lap-times)<br>
<br>
these options are active:<br>
* Serve penalty<br>
* Driver change<br>
<br>
The fix options (bodywork, suspension and so on) will be ignored, even if active.<br>
<br>
The pit-stop execution will try to set all the pit-options, even if it failed with some of the options.<br>
If you want to manually alter any pit-menu settings, let the pit-stop execution finish (the request box button stops blinking).
