# game gui

## design goals
* should be easy to use, preferably with a wysiwyg editor
* should handle both hud, in game panels/systems and meny
* adding theme, interactions and juice should be easy and without trouble

## contenders
We don't have to pick one, could use more than one and with combinations

### immediate mode with hot reload
* Use some sort of [rect cut](https://halt.software/dead-simple-layouts/) layout
* Can compose components easily
* immediate mode or react inspired interaction
* seperate fire-and-forget layer for basic interactions?

### quake/winforms inspired with specialized cpp widgets
* basic widgets like button and keybind are implemented in cpp
* a ui consists of 1-many layers with 0-many widgets
  a layer has a design size and resize algorithm, algoritm is either box or extended with a corner/center alignment, one would be box-center or extend top-right
  widgets have winforms anchoring and size setup and is easily design in a wysiwyg ui
* timeline with command support like quake4/doom ui to add basic animation
* all properties are smoothable/tweenable with dotween-like actions unless controlled by timeline to allow fire and forget functions

### full on parent/child tree with layout algorithm and unity-anchoring
* unity: [Making UI That Looks Good In Unity](https://www.youtube.com/watch?v=HwdweCX5aMI) and [How To Get A Better Grid Layout in Unity](https://www.youtube.com/watch?v=CGsEJToeXmA)
* Canvas is base, each component has a rect: `left`, `top`, `right`, `bottom`, `anchor_min_xy`, `anchor_max_xy`, `rotation`, `scale`
* Empy/Panels can have layout
 - `horizontal group` and `vertical group`: setting to expand spacing, resize elements, stack elements or resize [min/preferred height] info from components
 - `layout elements`
 - `content size fitter`


## layout:
* winforms
* java swing
* constraint layout: [Programming a GUI Library for my New Game - YouTube](https://www.youtube.com/watch?v=d5ttbNtpgi4)
```java
public static void createUi() {
	Constraints constraints = new Constraints();
	constraints.setX(new CenterContraint());
	constraints.setY(new PixelContraints(20)); // from top
	contraints.setWidth(new RelativeConstraint(0.1f)); // 10% of available
	constraints.setHeight(new AspectContraint(1.0f));

	Component display = Master.getContainer();
	Component element = new Block(Colors.DarkGrey);
	display.add(element, contraints);
}
```

## animation between slides
Driver for animation: [Finishing the GUI Library for my City-Builder Game - YouTube](https://www.youtube.com/watch?v=80b2oqecN4s)
```java
Transition slide_and_fade = new Transition()
	.xDriver(new SlideTransition(0, -0.15f, TIME))
	.alphaDriver(new SlideTransition(1, 0, TIME))
	;

UiBlock button = new UiBlock();
add(button, position);

float delay = button_number * 0.006f;
button.getAnimator().addTransition(slide_and_fade, delay);
// transition is used when show/hide is applied

// false: <-
// true: ->
button.getAnimator().applyAnimation(slide_and_fade, 0.0f, true);
```

Perhaps should be some "global" system "merged" with dotween like calls

## change states
Smoother (as a replacement/more features for Tweenable in euph?)
```java
SmoothAngle angle;
angle.force(new_angle);
angle.set_target(new_angle);
angle.update(dt);
set_angle(angle.get());
```

