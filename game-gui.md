# game gui

## layout:
* rect cut
* winforms
* unity
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

