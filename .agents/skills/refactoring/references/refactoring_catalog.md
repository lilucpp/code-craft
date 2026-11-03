# 68 种经典重构手法全集 (Refactoring Catalog)

> 整理自马丁·福勒《重构：改善既有代码的设计》第6~11章。


# 第6章 重新组织函数 (Composing Methods)

## 6.1 Extract Method（提炼函数）

你有一段代码可以被组织在一起并独立出来。

将这段代码放进一个独立函数中，并让函数名称解释该函数的用途。

void printOwing(double amount){
printBanner();
// print details
System.out.println("name:" + _name);
System.out.println("amount" + amount);
}

void printOwing(double amount){
printBanner();
printDetails(amount);
}
void printDetails(double amount){
System.out.println("name:" + _name);
System.out.println("amount" + amount);
}

动机

Extract Method (110)是我最常用的重构手法之一。当我看见一个过长的函数或者一段需要注释才能让人理解用途的代码，我就会将这段代码放进一个独立函数中。

有几个原因造成我喜欢简短而命名良好的函数。首先，如果每个函数的粒度都很小，那么函数被复用的机会就更大；其次，这会使高层函数读起来就像一系列注释；再次，如果函数都是细粒度，那么函数的覆写也会更容易些。

的确，如果你习惯看大型函数，恐怕需要一段时间才能适应这种新风格。而且只有当你能给小型函数很好地命名时，它们才能真正起作用，所以你需要在函数名称上下点功夫。人们有时会问我，一个函数多长才算合适？在我看来，长度不是问题，关键在于函数名称和函数本体之间的语义距离。如果提炼可以强化代码的清晰度，那就去做，就算函数名称比提炼出来的代码还长也无所谓。

做法

创造一个新函数，根据这个函数的意图来对它命名（以它“做什么”来命名，而不是以它“怎样做”命名）。

即使你想要提炼的代码非常简单，例如只是一条消息或一个函数调用，只要新函数的名称能够以更好的方式昭示代码意图，你也应该提炼它。但如果你想不出一个更有意义的名称，就别动。

将提炼出的代码从源函数复制到新建的目标函数中。

仔细检查提炼出的代码，看看其中是否引用了“作用域限于源函数”的变量（包括局部变量和源函数参数）。

检查是否有“仅用于被提炼代码段”的临时变量。如果有，在目标函数中将它们声明为临时变量。

检查被提炼代码段，看看是否有任何局部变量的值被它改变。如果一个临时变量值被修改了，看看是否可以将被提炼代码段处理为一个查询，并将结果赋值给相关变量。如果很难这样做，或如果被修改的变量不止一个，你就不能仅仅将这段代码原封不动地提炼出来。你可能需要先使用Split Temporary Variable (128)，然后再尝试提炼。也可以使用Replace Temp with Query (120)将临时变量消灭掉（请看“范例”中的讨论）。

将被提炼代码段中需要读取的局部变量，当作参数传给目标函数。

处理完所有局部变量之后，进行编译。

在源函数中，将被提炼代码段替换为对目标函数的调用。

如果你将任何临时变量移到目标函数中，请检查它们原本的声明式是否在被提炼代码段的外围。如果是，现在你可以删除这些声明式了。

编译，测试。

范例：无局部变量

在最简单的情况下，Extract Method (110)易如反掌。请看下列函数：

void printOwing(){

Enumeration e = _orders.elements();

double outstanding = 0.0;

// print banner

System.out.println("**************************");

System.out.println("***** Customer Owes ******");

System.out.println("**************************");

// calculate outstanding

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

outstanding += each.getAmount();

}

// print details

System.out.println("name:" + _name);

System.out.println("amount" + outstanding);

}

我们可以轻松提炼出“打印横幅”的代码。我只需要剪切、粘贴、再插入一个函数调用动作就行了：

void printOwing(){

Enumeration e = _orders.elements();

double outstanding = 0.0;

printBanner();

// calculate outstanding

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

outstanding += each.getAmount();

}

// print details

System.out.println("name:" + _name);

System.out.println("amount" + outstanding);

}

void printBanner(){

// print banner

System.out.println("**************************");

System.out.println("***** Customer Owes ******");

System.out.println("**************************");

}

范例：有局部变量

果真这么简单，这个重构手法的困难点在哪里？是的，就在局部变量，包括传进源函数的参数和源函数所声明的临时变量。局部变量的作用域仅限于源函数，所以当我使用Extract Method (110)时，必须花费额外功夫去处理这些变量。某些时候它们甚至可能妨碍我，使我根本无法进行这项重构。

局部变量最简单的情况是：被提炼代码段只是读取这些变量的值，并不修改它们。这种情况下我可以简单地将它们当作参数传给目标函数。所以如果我面对下列函数：

void printOwing(){

Enumeration e = _orders.elements();

double outstanding = 0.0;

printBanner();

// calculate outstanding

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

outstanding += each.getAmount();

}

// print details

System.out.println("name:" + _name);

System.out.println("amount" + outstanding);

}

就可以将“打印详细信息”这一部分提炼为带一个参数的函数：

void printOwing(){

Enumeration e = _orders.elements();

double outstanding = 0.0;

printBanner();

// calculate outstanding

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

outstanding += each.getAmount();

}

printDetails(outstanding);

}

void printDetails(double outstanding){

System.out.println("name:" + _name);

System.out.println("amount" + outstanding);

}

必要的话，你可以用这种手法处理多个局部变量。

如果局部变量是个对象，而被提炼代码段调用了会对该对象造成修改的函数，也可以如法炮制。你同样只需将这个对象作为参数传递给目标函数即可。只有在被提炼代码段真的对一个局部变量赋值的情况下，你才必须采取其他措施。

范例：对局部变量再赋值

如果被提炼代码段对局部变量赋值，问题就变得复杂了。这里我们只讨论临时变量的问题。如果你发现源函数的参数被赋值，应该马上使用Remove Assignments to Parameters (131)。

被赋值的临时变量也分两种情况。较简单的情况是：这个变量只在被提炼代码段中使用。果真如此，你可以将这个临时变量的声明移到被提炼代码段中，然后一起提炼出去。另一种情况是：被提炼代码段之外的代码也使用了这个变量。这又分为两种情况：如果这个变量在被提炼代码段之后未再被使用，你只需直接在目标函数中修改它就可以了；如果被提炼代码段之后的代码还使用了这个变量，你就需要让目标函数返回该变量改变后的值。我以下列代码说明这几种不同情况：

void printOwing(){

Enumeration e = _orders.elements();

double outstanding = 0.0;

printBanner();

// calculate outstanding

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

outstanding += each.getAmount();

}

printDetails(outstanding);

}

现在我把“计算”代码提炼出来：

void printOwing(){

printBanner();

double outstanding = getOutstanding();

printDetails(outstanding);

}

double getOutstanding(){

Enumeration e = _orders.elements();

double outstanding = 0.0;

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

outstanding += each.getAmount();

}

return outstanding;

}

Enumeration变量e只在被提炼代码段中用到，所以可以将它整个搬到新函数中。double变量outstanding在被提炼代码段内外都被用到，所以必须让提炼出来的新函数返回它。编译测试完成后，我就把回传值改名，遵循我的一贯命名原则：

double getOutstanding(){

Enumeration e = _orders.elements();

double result = 0.0;

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

result+ = each.getAmount();

}

return result;

}

本例中的outstanding变量只是很单纯地被初始化为一个明确初值，所以我可以只在新函数中对它初始化。如果代码还对这个变量做了其他处理，就必须将它的值作为参数传给目标函数。对于这种变化，最初代码可能是这样：

void printOwing(double previousAmount){

Enumeration e = _orders.elements();

double outstanding = previousAmount * 1.2;

printBanner();

// calculate outstanding

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

outstanding += each.getAmount();

}

printDetails(outstanding);

}

提炼后的代码可能是这样：

void printOwing(double previousAmount){

double outstanding = previousAmount * 1.2;

printBanner();

outstanding = getOutstanding(outstanding);

printDetails(outstanding);

}

double getOutstanding(double initialValue){

double result = initialValue;

Enumeration e = _orders.elements();

while (e.hasMoreElements()){

Order each = (Order)e.nextElement();

result += each.getAmount();

}

return result;

}

编译并测试后，我再将变量outstanding的初始化过程整理一下：

void printOwing(double previousAmount){

printBanner();

double outstanding = getOutstanding(previousAmount * 1.2);

printDetails(outstanding);

}

这时候，你可能会问：“如果需要返回的变量不止一个，又该怎么办呢？”

有几种选择。最好的选择通常是：挑选另一块代码来提炼。我比较喜欢让每个函数都只返回一个值，所以会安排多个函数，用以返回多个值。如果你使用的语言支持“出参数”（output parameter），可以使用它们带回多个回传值。但我还是尽可能选择单一返回值。

临时变量往往为数众多，甚至会使提炼工作举步维艰。这种情况下，我会尝试先运用Replace Temp with Query (120)减少临时变量。如果即使这么做了提炼依旧困难重重，我就会动用Replace Method with Method Object (135)，这个重构手法不在乎代码中有多少临时变量，也不在乎你如何使用它们。

---

## 6.2 Inline Method（内联函数）

一个函数的本体与名称同样清楚易懂。

在函数调用点插入函数本体，然后移除该函数。

int getRating(){
return (moreThanFiveLateDeliveries())? 2 : 1;
}
boolean moreThanFiveLateDeliveries(){
return _numberOfLateDeliveries > 5;
}

int getRating(){
return (_numberOfLateDeliveries > 5)? 2 : 1;
}

动机

本书经常以简短的函数表现动作意图，这样会使代码更清晰易读。但有时候你会遇到某些函数，其内部代码和函数名称同样清晰易读。也可能你重构了该函数，使得其内容和其名称变得同样清晰。果真如此，你就应该去掉这个函数，直接使用其中的代码。间接性可能带来帮助，但非必要的间接性总是让人不舒服。

另一种需要使用Inline Method (117)的情况是：你手上有一群组织不甚合理的函数。你可以将它们都内联到一个大型函数中，再从中提炼出组织合理的小型函数。Kent Beck发现，实施Replace Method with Method Object (135)之前先这么做，往往可以获得不错的效果。你可以把所要的函数（有着你要的行为）的所有调用对象的函数内容都内联到函数对象中。比起既要移动一个函数、又要移动它所调用的其他所有函数，将整个大型函数作为整体来移动会比较简单。

如果别人使用了太多间接层，使得系统中的所有函数都似乎只是对另一个函数的简单委托，造成我在这些委托动作之间晕头转向，那么我通常都会使用Inline Method (117)。当然，间接层有其价值，但不是所有间接层都有价值。试着使用内联手法，我可以找出那些有用的间接层，同时将那些无用的间接层去除。

做法

检查函数，确定它不具多态性。

如果子类继承了这个函数，就不要将此函数内联，因为子类无法覆写一个根本不存在的函数。

找出这个函数的所有被调用点。

将这个函数的所有被调用点都替换为函数本体。

编译，测试。

删除该函数的定义。

被我这样一写，Inline Method (117)似乎很简单。但情况往往并非如此。对于递归调用、多返回点、内联至另一个对象中而该对象并无提供访问函数……每一种情况我都可以写上好几页。我之所以不写这些特殊情况，原因很简单：如果你遇到了这样的复杂情况，那么就不应该使用这个重构手法。

---

## 6.3 Inline Temp（内联临时变量）

你有一个临时变量，只被一个简单表达式赋值一次，而它妨碍了其他重构手法。

将所有对该变量的引用动作，替换为对它赋值的那个表达式自身。

double basePrice = anOrder.basePrice();
return (basePrice > 1000)

return (anOrder.basePrice()> 1000)

动机

Inline Temp (119)多半是作为Replace Temp with Query (120)的一部分使用的，所以真正的动机出现在后者那儿。唯一单独使用Inline Temp (119)的情况是：你发现某个临时变量被赋予某个函数调用的返回值。一般来说，这样的临时变量不会有任何危害，可以放心地把它留在那儿。但如果这个临时变量妨碍了其他的重构手法，例如Extract Method (110)，你就应该将它内联化。

做法

检查给临时变量赋值的语句，确保等号右边的表达式没有副作用。

如果这个临时变量并未被声明为final，那就将它声明为final，然后编译。

这可以检查该临时变量是否真的只被赋值一次。

找到该临时变量的所有引用点，将它们替换为“为临时变量赋值”的表达式。

每次修改后，编译并测试。

修改完所有引用点之后，删除该临时变量的声明和赋值语句。

编译，测试。

---

## 6.4 Replace Temp with Query（以查询取代临时变量）

你的程序以一个临时变量保存某一表达式的运算结果。

将这个表达式提炼到一个独立函数中。将这个临时变量的所有引用点替换为对新函数的调用。此后，新函数就可被其他函数使用。

double basePrice = _quantity * _itemPrice;
if (basePrice > 1000)
return basePrice * 0.95;
else
return basePrice * 0.98;

if (basePrice() > 1000)
return basePrice() * 0.95;
else
return basePrice() * 0.98;
...
double basePrice() {
return _quantity * _itemPrice;
}

动机

临时变量的问题在于：它们是暂时的，而且只能在所属函数内使用。由于临时变量只在所属函数内可见，所以它们会驱使你写出更长的函数，因为只有这样你才能访问到需要的临时变量。如果把临时变量替换为一个查询，那么同一个类中的所有函数都将可以获得这份信息。这将带给你极大帮助，使你能够为这个类编写更清晰的代码。

Replace Temp with Query (120)往往是你运用Extract Method (110)之前必不可少的一个步骤。局部变量会使代码难以被提炼，所以你应该尽可能把它们替换为查询式。

这个重构手法较为简单的情况是：临时变量只被赋值一次，或者赋值给临时变量的表达式不受其他条件影响。其他情况比较棘手，但也有可能发生。你可能需要先运用Split Temporary Variable (128)或Separate Query from Modifier (279)使情况变得简单一些，然后再替换临时变量。如果你想替换的临时变量是用来收集结果的（例如循环中的累加值），就需要将某些程序逻辑（例如循环）复制到查询函数去。

做法

首先是简单情况：

找出只被赋值一次的临时变量。

如果某个临时变量被赋值超过一次，考虑使用Split Temporary Variable (128)将它分割成多个变量。

将该临时变量声明为final。

编译。

这可确保该临时变量的确只被赋值一次。

将“对该临时变量赋值”之语句的等号右侧部分提炼到一个独立函数中。

首先将函数声明为private。日后你可能会发现有更多类需要使用它，那时放松对它的保护也很容易。

确保提炼出来的函数无任何副作用，也就是说该函数并不修改任何对象内容。如果它有副作用，就对它进行Separate Query from Modifler (279)。

编译，测试。

在该临时变量身上实施Inline Temp (119)。

我们常常使用临时变量保存循环中的累加信息。在这种情况下，整个循环都可以被提炼为一个独立函数，这也使原本的函数可以少掉几行扰人的循环逻辑。有时候，你可能会在一个循环中累加好几个值，就像本书第26页的例子那样。这种情况下你应该针对每个累加值重复一遍循环，这样就可以将所有临时变量都替换为查询。当然，循环应该很简单，复制这些代码时才不会带来危险。

运用此手法，你可能会担心性能问题。和其他性能问题一样，我们现在不管它，因为它十有八九根本不会造成任何影响。若是性能真的出了问题，你也可以在优化时期解决它。代码组织良好，你往往能够发现更有效的优化方案：如果没有进行重构，好的优化方案就可能与你失之交臂。如果性能实在太糟糕，要把临时变量放回去也是很容易的。

范例

首先，我从一个简单函数开始：

double getPrice(){

int basePrice = _quantity * _itemPrice;

double discountFactor;

if (basePrice > 1000)discountFactor = 0.95;

else discountFactor = 0.98;

return basePrice * discountFactor;

}

我希望将两个临时变量都替换掉。当然，每次一个。

尽管这里的代码十分清楚，我还是先把临时变量声明为final，检查它们是否的确只被赋值一次：

double getPrice(){

final int basePrice = _quantity * _itemPrice;

final double discountFactor;

if (basePrice > 1000)discountFactor = 0.95;

else discountFactor = 0.98;

return basePrice * discountFactor;

}

这么一来，如果有任何问题，编译器就会警告我。之所以先做这件事，因为如果临时变量不只被赋值一次，我就不该进行这项重构。接下来开始替换临时变量，每次一个。首先，我把赋值动作的右侧表达式提炼出来：

double getPrice(){

final int basePrice = basePrice();

final double discountFactor;

if (basePrice > 1000)discountFactor = 0.95;

else discountFactor = 0.98;

return basePrice * discountFactor;

}

private int basePrice(){

return _quantity * _itemPrice;

}

编译并测试，然后开始使用Inline Temp (119)。首先把临时变量basePrice的第一个引用点替换掉：

double getPrice(){

final int basePrice = basePrice();

final double discountFactor;

if (basePrice()> 1000)discountFactor = 0.95;

else discountFactor = 0.98;

return basePrice * discountFactor;

}

编译、测试、下一个（听起来像在指挥人们跳乡村舞蹈一样）。由于“下一个”已经是basePrice的最后一个引用点，所以我把basePrice临时变量的声明式一并去掉：

double getPrice(){

final double discountFactor;

if (basePrice()> 1000)discountFactor = 0.95;

else discountFactor = 0.98;

return basePrice()* discountFactor;

}

搞定basePrice之后，我再以类似办法提炼出discountFactor()：

double getPrice(){

final double discountFactor = discountFactor();

return basePrice()* discountFactor;

}

private double discountFactor(){

if (basePrice()> 1000)return 0.95;

else return 0.98;

}

你看，如果我没有把临时变量basePrice替换为一个查询式，将多么难以提炼discountFactor()!

最终，getPrice()变成了这样：

double getPrice(){

return basePrice()* discountFactor();

}

---

## 6.5 Introduce Explaining Variable（引入解释性变量）

你有一个复杂的表达式。

将该复杂表达式（或其中一部分）的结果放进一个临时变量，以此变量名称来解释表达式用途。

if ((platform.toUpperCase().indexOf("MAC")> -1)&&
(browser.toUpperCase().indexOf("IE")> -1)&&
wasInitialized()&& resize > 0)
{
// do something
}

final boolean isMacOs　　　= platform.toUpperCase().indexOf("MAC")> -1;
final boolean isIEBrowser = browser.toUpperCase().indexOf("IE")> -1;
final boolean wasResized　 = resize > 0;
if (isMacOs && isIEBrowser && wasInitialized()&& wasResized){
// do something
}

动机

表达式有可能非常复杂而难以阅读。这种情况下，临时变量可以帮助你将表达式分解为比较容易管理的形式。

在条件逻辑中，Introduce Explaining Variable (124)特别有价值：你可以用这项重构将每个条件子句提炼出来，以一个良好命名的临时变量来解释对应条件子句的意义。使用这项重构的另一种情况是，在较长算法中，可以运用临时变量来解释每一步运算的意义。

Introduce Explaining Variable (124)是一个很常见的重构手法，但我得承认，我并不常用它。我几乎总是尽量使用Extract Method (110)来解释一段代码的意义。毕竟临时变量只在它所处的那个函数中才有意义，局限性较大，函数则可以在对象的整个生命中都有用，并且可被其他对象使用。但有时候，当局部变量使Extract Method (110)难以进行时，我就使用Introduce Explaining Variable (124)。

做法

声明一个final临时变量，将待分解之复杂表达式中的一部分动作的运算结果赋值给它。

将表达式中的“运算结果”这一部分，替换为上述临时变量。

如果被替换的这一部分在代码中重复出现，你可以每次一个，逐一替换。

编译，测试。

重复上述过程，处理表达式的其他部分。

范例

我们从一个简单计算开始：

double price(){

// price is base price - quantity discount + shipping

return _quantity * _itemPrice -

Math.max(0,_quantity - 500)* _itemPrice * 0.05 +

Math.min(_quantity * _itemPrice * 0.1,100.0);

}

这段代码还算简单，不过我可以让它变得更容易理解。首先我发现，底价（base price）等于数量（quantity）乘以单价（item price）。于是，我把这一部分计算的结果放进一个临时变量中：

double price(){

// price is base price - quantity discount + shipping

final double basePrice = _quantity * _itemPrice;

return basePrice ñ

Math.max(0,_quantity - 500)* _itemPrice * 0.05 +

Math.min(_quantity * _itemPrice * 0.1,100.0);

}

稍后也用上了“数量乘以单价”运算结果，所以我同样将它替换为basePrice临时变量：

double price(){

// price is base price - quantity discount + shipping

final double basePrice = _quantity * _itemPrice;

return basePrice ñ

Math.max(0,_quantity - 500)* _itemPrice * 0.05 +

Math.min(basePrice * 0.1,100.0);

}

然后，我将批发折扣（quantity discount）的计算提炼出来，将结果赋予临时变量

double price(){

// price is base price - quantity discount + shipping

final double basePrice = _quantity * _itemPrice;

final double quantityDiscount = Math.max(0,_quantity - 500)* _itemPrice * 0.05;

return basePrice - quantityDiscount +

Math.min(basePrice * 0.1,100.0);

}

最后，我再把运费（shipping）计算提炼出来，将运算结果赋予临时变量shipping。同时我还可以删掉代码中的注释，因为现在代码已经可以完美表达自己的意义了：

double price(){

final double basePrice = _quantity * _itemPrice;

final double quantityDiscount = Math.max(0,_quantity - 500)* _itemPrice * 0.05;

final double shipping = Math.min(basePrice * 0.1,100.0);

return basePrice - quantityDiscount + shipping;

}

运用Extract Method处理上述范例

面对上述代码，我通常不会以临时变量来解释其动作意图，我更喜欢使用Extract Method (110)。让我们回到起点：

double price(){

// price is base price - quantity discount + shipping

return _quantity * _itemPrice ñ

Math.max(0,_quantity - 500)* _itemPrice * 0.05+

Math.min(_quantity * _itemPrice * 0.1,100.0);

}

这一次我把底价计算提炼到一个独立函数中：

double price(){

// price is base price - quantity discount + shipping

return basePrice()-

Math.max(0,_quantity - 500)* _itemPrice * 0.05 +

Math.min(basePrice()* 0.1,100.0);

}

private double basePrice(){

return _quantity * _itemPrice;

}

我继续提炼，每次提炼出一个新函数。最后得到下列代码：

double price(){

return basePrice()- quantityDiscount()+ shipping();

}

private double quantityDiscount(){

return Math.max(0,_quantity - 500)* _itemPrice * 0.05;

}

private double shipping(){

return Math.min(basePrice()* 0.1,100.0);

}

private double basePrice(){

return _quantity * _itemPrice;

}

我比较喜欢使用Extract Method (110)，因为同一对象中的任何部分，都可以根据自己的需要取用这些提炼出来的函数。一开始我会把这些新函数声明为private；如果其他对象也需要它们，我可以轻易释放这些函数的访问限制。我还发现，Extract Method (110)的工作量通常并不比Introduce Explaining Variable (124)来得大。

那么，应该在什么时候使用Introduce Explaining Variable (124)呢？答案是：在Extract Method (110)需要花费更大工作量时。如果我要处理的是一个拥有大量局部变量的算法，那么使用Extract Method (110)绝非易事。这种情况下就会使用Introduce Explaining Variable (124)来理清代码，然后再考虑下一步该怎么办。搞清楚代码逻辑之后，我总是可以运用Replace Temp with Query (120)把中间引入的那些解释性临时变量去掉。况且，如果我最终使用Replace Method with Method Object (135)，那么中间引入的那些解释性临时变量也有其价值。

---

## 6.6 Split Temporary Variable（分解临时变量）

你的程序有某个临时变量被赋值超过一次，它既不是循环变量，也不被用于收集计算结果。

针对每次赋值，创造一个独立、对应的临时变量。

double temp = 2 * (_height + _width);
System.out.println (temp);
temp = _height * _width;
System.out.println (temp);

final double perimeter = 2 * (_height + _width);
System.out.println (perimeter);
final double area = _height * _width;
System.out.println (area);

动机

临时变量有各种不同用途，其中某些用途会很自然地导致临时变量被多次赋值。“循环变量”和“结果收集变量”就是两个典型例子：循环变量（loop variable） [Beck]会随循环的每次运行而改变（例如for(int i=0; i<10; i++)语句中的i）；结果收集变量（collecting temporary variable）[Beck]负责将“通过整个函数的运算”而构成的某个值收集起来。

除了这两种情况，还有很多临时变量用于保存一段冗长代码的运算结果，以便稍后使用。这种临时变量应该只被赋值一次。如果它们被赋值超过一次，就意味它们在函数中承担了一个以上的责任。如果临时变量承担多个责任，它就应该被替换（分解）为多个临时变量，每个变量只承担一个责任。同一个临时变量承担两件不同的事情，会令代码阅读者糊涂。

做法

在待分解临时变量的声明及其第一次被赋值处，修改其名称。

如果稍后之赋值语句是[i=i+某表达式]形式，就意味这是个结果收集变量，那么就不要分解它。结果收集变量的作用通常是累加、字符串接合、写入流或者向集合添加元素。

将新的临时变量声明为final。

以该临时变量的第二次赋值动作为界，修改此前对该临时变量的所有引用点，让它们引用新的临时变量。

在第二次赋值处，重新声明原先那个临时变量。

编译，测试。

逐次重复上述过程。每次都在声明处对临时变量改名，并修改下次赋值之前的引用点。

范例

下面范例中我要计算一个苏格兰布丁运动的距离。在起点处，静止的苏格兰布丁会受到一个初始力的作用而开始运动。一段时间后，第二个力作用于布丁，让它再次加速。根据牛顿第二定律，我可以这样计算布丁运动的距离：

double getDistanceTravelled(int time){

double result;

double acc = _primaryForce / _mass;

int primaryTime = Math.min(time,_delay);

result = 0.5 * acc * primaryTime * primaryTime;

int secondaryTime = time - _delay;

if (secondaryTime > 0){

double primaryVel = acc * _delay;

acc = (_primaryForce + _secondaryForce)/ _mass;

result += primaryVel * secondaryTime + 0.5 * acc * secondaryTime

* secondaryTime;

}

return result;

}

真是个丑陋的小东西。注意观察此例中的acc变量如何被赋值两次。acc变量有两个责任：第一是保存第一个力造成的初始加速度；第二是保存两个力共同造成的加速度。这就是我想要分解的东西。

首先，我在函数开始处修改这个临时变量的名称，并将新的临时变量声明为final。接着，我把第二次赋值之前对acc变量的所有引用点，全部改用新的临时变量。最后，我在第二次赋值处重新声明acc变量：

double getDistanceTravelled(int time){

double result;

final double primaryAcc = _primaryForce / _mass;

int primaryTime = Math.min(time,_delay);

result = 0.5 * primaryAcc * primaryTime * primaryTime;

int secondaryTime = time - _delay;

if (secondaryTime > 0){

double primaryVel = primaryAcc * _delay;

double acc = (_primaryForce + _secondaryForce)/ _mass;

result += primaryVel * secondaryTime + 0.5 * acc * secondaryTime

* secondaryTime;

}

return result;

}

新的临时变量的名称指出，它只承担原先acc变量的第一个责任。我将它声明为final，确保它只被赋值一次。然后，我在原先acc变量第二次被赋值处重新声明acc。现在，重新编译并测试，一切都应该没有问题。

然后，我继续处理acc临时变量的第二次赋值。这次我把原先的临时变量完全删掉，代之以一个新的临时变量。新变量的名称指出，它只承担原先acc变量的第二个责任：

double getDistanceTravelled(int time){

double result;

final double primaryAcc = _primaryForce / _mass;

int primaryTime = Math.min(time,_delay);

result = 0.5 * primaryAcc * primaryTime * primaryTime;

int secondaryTime = time - _delay;

if (secondaryTime > 0){

double primaryVel = primaryAcc * _delay;

final double secondaryAcc = (_primaryForce + _secondaryForce)/ _mass;

result += primaryVel * secondaryTime + 0.5 *

secondaryAcc * secondaryTime * secondaryTime;

}

return result;

}

现在，这段代码肯定可以让你想起更多其他重构手法。尽情享受吧。（我敢保证，这比吃苏格兰布丁强多了——你知道他们都在里面放了些什么东西吗？
[1]

 ）

---

## 6.7 Remove Assignments to Parameters（移除对参数的赋值）

代码对一个参数进行赋值。

以一个临时变量取代该参数的位置。

int discount (int inputVal,int quantity,int yearToDate){
if (inputVal > 50)inputVal -= 2;

int discount (int inputVal,int quantity,int yearToDate){
int result = inputVal;
if (inputVal > 50)result -= 2;

动机

首先，我要确定大家都清楚“对参数赋值”这个说法的意思。如果你把一个名为foo的对象作为参数传给某个函数，那么“对参数赋值”意味改变foo，使它引用另一个对象。如果你在“被传入对象”身上进行什么操作，那没问题，我也总是这样干。我只针对“foo被改而指向另一个对象”这种情况来讨论：

void aMethod(Object foo){

foo.modifyInSomeWay(); // that's OK

foo = anotherObject; // trouble and despair will follow you

我之所以不喜欢这样的做法，因为它降低了代码的清晰度，而且混用了按值传递和按引用传递这两种参数传递方式。Java只采用按值传递方式（稍后讨论），我们的讨论也正是基于这一点。

在按值传递的情况下，对参数的任何修改，都不会对调用端造成任何影响。那些用过按引用传递方式的人可能会在这一点上犯糊涂。

另一个让人糊涂的地方是函数本体内。如果你只以参数表示“被传递进来的东西”，那么代码会清晰得多，因为这种用法在所有语言中都表现出相同语义。

在Java中，不要对参数赋值：如果你看到手上的代码已经这样做了，请使用Remove Assignments to Parameters (131)。

当然，面对那些使用“出参数”的语言，你不必遵循这条规则。不过在那些语言中我会尽量少用出参数。

做法

建立一个临时变量，把待处理的参数值赋予它。

以“对参数的赋值”为界，将其后所有对此参数的引用点，全部替换为“对此临时变量的引用”。

修改赋值语句，使其改为对新建之临时变量赋值。

编译，测试。

如果代码的语义是按引用传递的，请在调用端检查调用后是否还使用了这个参数。也要检查有多少个按引用传递的参数被赋值后又被使用。请尽量只以return方式返回一个值。如果需要返回的值不止一个，看看可否把需返回的大堆数据变成单一对象，或干脆为每个返回值设计对应的一个独立函数。

范例

我从下列这段简单代码开始：

int discount (int inputVal,int quantity,int yearToDate){

if (inputVal > 50)inputVal -= 2;

if (quantity > 100)inputVal -= 1;

if (yearToDate > 10000)inputVal -= 4;

return inputVal;

}

以临时变量取代对参数的赋值动作，得到下列代码：

int discount (int inputVal,int quantity,int yearToDate){

int result = inputVal;

if (inputVal > 50)result -= 2;

if (quantity > 100)result -= 1;

if (yearToDate > 10000)result -= 4;

return result;

}

还可以为参数加上关键词final，从而强制它遵循“不对参数赋值”这一惯例：

int discount (final int inputVal,final int quantity,final int yearToDate){

int result = inputVal;

if (inputVal > 50)result -= 2;

if (quantity > 100)result -= 1;

if (yearToDate > 10000)result -= 4;

return result;

}

不过我得承认，我并不经常使用final来修饰参数，因为我发现，对于提高短函数的清晰度，这个办法并无太大帮助。我通常会在较长的函数中使用它，让它帮助我检查参数是否被做了修改。

Java的按值传递

Java使用按值传递的函数调用方式，这常常造成许多人迷惑。在所有地点，Java都严格采用按值传递方式，所以下列程序：

class Param {

public static void main(String[] args){

int x = 5;

triple(x);

System.out.println("x after triple: " + x);

}

private static void triple(int arg){

arg = arg * 3;

System.out.println("arg in triple: " + arg);

}

}

会产生这样的输出：

arg in triple: 15

x after triple: 5

这段代码还不至于让人糊涂。但如果参数中传递的是对象，就可能把人弄迷糊了。如果我在程序中以Date对象表示日期，那么下列程序：

class Param {

public static void main(String[] args){

Date d1 = new Date("1 Apr 98");

nextDateUpdate(d1);

System.out.println("d1 after nextDay: " + d1);

Date d2 = new Date("1 Apr 98");

nextDateReplace(d2);

System.out.println("d2 after nextDay: " + d2);

}

private static void nextDateUpdate(Date arg){

arg.setDate(arg.getDate()+ 1);

System.out.println("arg in nextDay: " + arg);

}

private static void nextDateReplace(Date arg){

arg = new Date(arg.getYear(),arg.getMonth(),arg.getDate()+ 1);

System.out.println("arg in nextDay: " + arg);

}

}

产生的输出是：

arg in nextDay: Thu Apr 02 00:00:00 EST 1998

d1 after nextDay: Thu Apr 02 00:00:00 EST 1998

arg in nextDay: Thu Apr 02 00:00:00 EST 1998

d2 after nextDay: Wed Apr 01 00:00:00 EST 1998

从本质上说，对象的引用是按值传递的。因此，我可以修改参数对象的内部状态，但对参数对象重新赋值是没有意义的。

Java l.1及其后版本允许将参数标示为final，从而避免函数中对参数赋值。即使某个参数被标示为final，仍然可以修改它所指向的对象。我总是把参数视为final，但是我得承认，我很少在参数列表中这样标示它们。

---

## 6.8 Replace Method with Method Object（以函数对象取代函数）

你有一个大型函数，其中对局部变量的使用使你无法采用Extract Method (110)。

将这个函数放进一个单独对象中，如此一来局部变量就成了对象内的字段。然后你可以在同一个对象中将这个大型函数分解为多个小型函数。

class Order...
double price(){
double primaryBasePrice;
double secondaryBasePrice;
double tertiaryBasePrice;
// long computation;
...
}

动机

我在本书中不断向读者强调小型函数的优美动人。只要将相对独立的代码从大型函数中提炼出来，就可以大大提高代码的可读性。

但是，局部变量的存在会增加函数分解难度。如果一个函数之中局部变量泛滥成灾，那么想分解这个函数是非常困难的。Replace Temp with Query (120)可以助你减轻这一负担，但有时候你会发现根本无法拆解一个需要拆解的函数。这种情况下，你应该把手伸进工具箱的深处，祭出函数对象（method object）[Beck]这件法宝。

Replace Method with Method Object (135)会将所有局部变量都变成函数对象的字段。然后你就可以对这个新对象使用Extract Method (110)创造出新函数，从而将原本的大型函数拆解变短。

做法

我厚着脸皮从Kent Beck [Beck]那里偷来了下列做法。

建立一个新类，根据待处理函数的用途，为这个类命名。

在新类中建立一个final字段，用以保存原先大型函数所在的对象。我们将这个字段称为“源对象”。同时，针对原函数的每个临时变量和每个参数，在新类中建立一个对应的字段保存之。

在新类中建立一个构造函数，接收源对象及原函数的所有参数作为参数。

在新类中建立一个compute()函数。

将原函数的代码复制到compute()函数中。如果需要调用源对象的任何函数，请通过源对象字段调用。

编译。

将旧函数的函数本体替换为这样一条语句：“创建上述新类的一个新对象，而后调用其中的compute()函数”。

现在进行到很有趣的部分了。因为所有局部变量现在都成了字段，所以你可以任意分解这个大型函数，不必传递任何参数。

范例

如果要给这一重构手法找个合适例子，需要很长的篇幅。因此，我以一个不需要很长篇幅（也就是说并不完美）的例子展示这项重构。请不要问这个函数的逻辑是什么，这完全是我临时杜撰的产物。

Class Account

int gamma (int inputVal,int quantity,int yearToDate){

int importantValue1 = (inputVal * quantity)+ delta();

int importantValue2 = (inputVal * yearToDate)+ 100;

if ((yearToDate - importantValue1)> 100)

importantValue2 -= 20;

int importantValue3 = importantValue2 * 7;

// and so on.

return importantValue3 - 2 * importantValue1;

}

为了把这个函数变成一个函数对象，我首先需要声明一个新类。在此新类中我应该提供一个final字段用以保存源对象；对于函数的每一个参数和每一个临时变量，也以一个字段逐一保存。

class Gamma...

private final Account _account;

private int inputVal;

private int quantity;

private int yearToDate;

private int importantValue1;

private int importantValue2;

private int importantValue3;

按惯例，我通常会以下划线作为字段名称的前缀。但为了保持小步前进，我暂时先保留这些字段的原名。

接下来，加入一个构造函数：

Gamma (Account source,int inputValArg,int quantityArg,int yearToDateArg){

_account = source;

inputVal = inputValArg;

quantity = quantityArg;

yearToDate = yearToDateArg;

}

现在可以把原本的函数搬到compute()了。函数中任何调用Account类的地方，我都必须改而使用_account字段：

int compute (){

importantValue1 = (inputVal * quantity)+ _account.delta();

importantValue2 = (inputVal * yearToDate)+ 100;

if ((yearToDate - importantValue1)> 100)

importantValue2 -= 20;

int importantValue3 = importantValue2 * 7;

// and so on.

return importantValue3 - 2 * importantValue1;

}

然后，我修改旧函数，让它将它的工作委托给刚完成的这个函数对象：

int gamma (int inputVal,int quantity,int yearToDate){

return new Gamma(this,inputVal,quantity,yearToDate).compute();

}

这就是本项重构的基本原则。它带来的好处是：现在我可以轻松地对compute()函数采取Extract Method (110)，不必担心参数传递的问题。

int compute(){

importantValue1 = (inputVal * quantity)+ _account.delta();

importantValue2 = (inputVal * yearToDate)+ 100;

importantThing();

int importantValue3 = importantValue2 * 7;

// and so on.

return importantValue3 - 2 * importantValue1;

}

void importantThing(){

if ((yearToDate - importantValue1)> 100)

importantValue2 -= 20;

}

---

## 6.9 Substitute Algorithm（替换算法）

你想要把某个算法替换为另一个更清晰的算法。

将函数本体替换为另一个算法。

String foundPerson(String[] people){
for (int i = 0; i < people.length; i++){
if (people[i].equals("Don")){
return "Don";
}
if (people[i].equals("John")){
return "John";
}
if (people[i].equals("Kent")){
return "Kent";
}
}
return "";
}

String foundPerson(String[] people){
List candidates = Arrays.asList(new String[] { "Don","John","Kent" });
for (int i = 0; i < people.length; i++)
if (candidates.contains(people[i]))
return people[i];
return "";
}

动机

解决问题有好几种方法，我敢打赌其中某些方法会比另一些简单。算法也是如此。如果你发现做一件事可以有更清晰的方式，就应该以较清晰的方式取代复杂的方式。“重构”可以把一些复杂东西分解为较简单的小块，但有时你就必须壮士断腕，删掉整个算法，代之以较简单的算法。随着对问题有了更多理解，你往往会发现，在原先的做法之外，有更简单的解决方案，此时你就需要改变原先的算法。如果你开始使用程序库，而其中提供的某些功能/特性与你自己的代码重复，那么你也需要改变原先的算法。

有时候你会想要修改原先的算法，让它去做一件与原先略有差异的事。这时候你也可以先把原先的算法替换为一个较易修改的算法，这样后续的修改会轻松许多。

使用这项重构手法之前，请先确定自己已经尽可能分解了原先函数。替换一个巨大而复杂的算法是非常困难的，只有先将它分解为较简单的小型函数，然后你才能很有把握地进行算法替换工作。

做法

准备好另一个（替换用）算法，让它通过编译。

针对现有测试，执行上述的新算法。如果结果与原本结果相同，重构结束。

如果测试结果不同于原先，在测试和调试过程中，以旧算法为比较参照标准。

对于每个测试用例，分别以新旧两种算法执行，并观察两者结果是否相同。

这可以帮助你看到哪一个测试用例出现麻烦，以及出现了怎样的麻烦。

[1].
 苏格兰布丁（haggis）是一种苏格兰菜，把羊心等内脏装在羊胃里煮成。由于它被羊胃包成一个球体，因此可以像球一样踢来踢去，这就是本例的由来。“把羊心装在羊胃里煮成……”，呃，有些人难免对这道菜恶心，Martin Fowler想必是其中之一。——译者注

---


# 第7章 在对象之间搬移特性 (Moving Features Between Objects)

## 7.1 Move Method（搬移函数）

你的程序中，有个函数与其所驻类之外的另一个类进行更多交流：调用后者，或被后者调用。

在该函数最常引用的类中建立一个有着类似行为的新函数。将旧函数变成一个单纯的委托函数，或是将旧函数完全移除。

动机

“搬移函数”是重构理论的支柱。如果一个类有太多行为，或如果一个类与另一个类有太多合作而形成高度耦合，我就会搬移函数。通过这种手段，可以使系统中的类更简单，这些类最终也将更干净利落地实现系统交付的任务。

我常常会浏览类的所有函数，从中寻找这样的函数：使用另一个对象的次数比使用自己所驻对象的次数还多。一旦我移动了一些字段，就该做这样的检查。一旦发现有可能搬移的函数，我就会观察调用它的那一端、它调用的那一端，以及继承体系中它的任何一个重定义函数。然后，会根据“这个函数与哪个对象的交流比较多”，决定其移动路径。

这往往不是容易做出的决定。如果不能肯定是否应该移动一个函数，我就会继续观察其他函数。移动其他函数往往会让这项决定变得容易一些。有时候，即使你移动了其他函数，还是很难对眼下这个函数做出决定。其实这也没什么大不了的。如果真地很难做出决定，那么或许“移动这个函数与否”并不那么重要。所以，我会凭本能去做，反正以后总是可以修改的。

做法

检查源类中被源函数所使用的一切特性（包括字段和函数），考虑它们是否也该被搬移。

如果某个特性只被你打算搬移的那个函数用到，就应该将它一并搬移。如果另有其他函数使用了这个特性，你可以考虑将使用该特性的所有函数全都一并搬移。有时候，搬移一组函数比逐一搬移简单些。

检查源类的子类和超类，看看是否有该函数的其他声明。

如果出现其他声明，你或许无法进行搬移，除非目标类也同样表现出多态性。

在目标类中声明这个函数。

你可以为此函数选择一个新名称——对目标类更有意义的名称。

将源函数的代码复制到目标函数中。调整后者，使其能在新家中正常运行。

如果目标函数使用了源类中的特性，你得决定如何从目标函数引用源对象。如果目标类中没有相应的引用机制，就把源对象的引用当作参数，传给新建立的目标函数。

如果源函数包含异常处理，你得判断逻辑上应该由哪个类来处理这一异常。如果应该由源类来负责，就把异常处理留在原地。

编译目标类。

决定如何从源函数正确引用目标对象。

可能会有一个现成的字段或函数帮助你取得目标对象。如果没有，就看能否轻松建立一个这样的函数。如果还是不行，就得在源类中新建一个字段来保存目标对象。这可能是一个永久性修改，但你也可以让它是暂时的，因为后继的其他重构项目可能会把这个新建字段去掉。

修改源函数，使之成为一个纯委托函数。

编译，测试。

决定是否删除源函数，或将它当作一个委托函数保留下来。

如果你经常要在源对象中引用目标函数，那么将源函数作为委托函数保留下来会比较简单。

如果要移除源函数，请将源类中对源函数的所有调用，替换为对目标函数的调用。

你可以每修改一个引用点就编译并测试一次。也可以通过一次“查找/替换”改掉所有引用点，这通常简单一些。

编译，测试。

范例

我用一个表示“账户”的Account类来说明这项重构：

class Account...

double overdraftCharge(){

if (_type.isPremium()){

double result = 10;

if (_daysOverdrawn > 7)result += (_daysOverdrawn - 7)* 0.85;

return result;

}

else return _daysOverdrawn * 1.75;

}

double bankCharge(){

double result = 4.5;

if (_daysOverdrawn > 0)result += overdraftCharge();

return result;

}

private AccountType _type;

private int _daysOverdrawn;

假设有几种新账户，每一种都有自己的“透支金额计费规则”。我希望将overdraftCharge()搬移到AccountType类去。

第一步要做的是：观察被overdraftCharge()使用的每一项特性，考虑是否值得将它们与overdraftCharge()一起移动。此例之中我需要让_daysOver-drawn字段留在Account类，因为这个值会随不同种类的账户而变化。然后，我将overdraftCharge()函数码复制到AccountType中，并做相应调整。

class AccountType...

double overdraftCharge(int daysOverdrawn){

if (isPremium()){

double result = 10;

if (daysOverdrawn > 7)result += (daysOverdrawn - 7)* 0.85;

return result;

} else

return daysOverdrawn * 1.75;

}

在这个例子中，“调整”的意思是：(1)对于使用AccountType特性的语句，去掉_type；(2)想办法得到依旧需要的Account类特性。当我需要使用源类的特性时，有4种选择：(1)将这个特性也移到目标类；(2)建立或使用一个从目标类到源类的引用关系；(3)将源对象当作参数传给目标函数；(4)如果所需特性是个变量，将它当作参数传给目标函数。

本例中，我将_daysOverdrawn变量作为参数传给目标函数（上述(4)）。

调整目标函数使之通过编译，而后就可以将源函数的函数本体替换为一个简单的委托动作，然后编译并测试：

class Account...

double overdraftCharge(){

return _type.overdraftCharge(_daysOverdrawn);

}

我可以保留代码如今的样子，也可以删除源函数。如果决定删除，就得找出源函数的所有调用者，并将这些调用重新定向，改为调用Account的bankCharge()：

class Account...

double bankCharge(){

double result = 4.5;

if (_daysOverdrawn > 0)

result += _type.overdraftCharge(_daysOverdrawn);

return result;

}

所有调用点都修改完毕后，就可以删除源函数在Account中的声明了。我可以在每次删除之后编译并测试，也可以一次性批量完成。如果被搬移的函数不是private的，我还需要检查其他类是否使用了这个函数。在强类型语言中，删除源函数声明后，编译器会帮我发现任何遗漏。

此例之中被搬移函数只引用了一个字段，所以只需将这个字段作为参数传给目标函数就行了。如果被搬移函数调用了Account中的另一个函数，我就不能这么简单地处理。这种情况下必须将源对象传递给目标函数：

class AccountType...

double overdraftCharge(Account account){

if (isPremium()){

double result = 10;

if (account.getDaysOverdrawn()> 7)

result += (account.getDaysOverdrawn()- 7)* 0.85;

return result;

} else

return account.getDaysOverdrawn()* 1.75;

}

如果需要源类的多个特性，那么我也会将源对象传递给目标函数。不过如果目标函数需要太多源类特性，就得进一步重构。通常这种情况下，我会分解目标函数，并将其中一部分移回源类。

---

## 7.2 Move Field（搬移字段）

在你的程序中，某个字段被其所驻类之外的另一个类更多地用到。

在目标类新建一个字段，修改源字段的所有用户，令它们改用新字段。

动机

在类之间移动状态和行为，是重构过程中必不可少的措施。随着系统的发展，你会发现自己需要新的类，并需要将现有的工作责任拖到新的类中。在这个星期看似合理而正确的设计决策，到了下个星期可能不再正确。这没问题。如果你从来没遇到这种情况，那才有问题。

如果我发现，对于一个字段，在其所驻类之外的另一个类中有更多函数使用了它，我就会考虑搬移这个字段。上述所谓“使用”可能是通过设值/取值函数间接进行的。我也可能移动该字段的用户（某个函数），这取决于是否需要保持接口不受变化。如果这些函数看上去很适合待在原地，我就选择搬移字段。

使用Extract Class (149)时，我也可能需要搬移字段。此时我会先搬移字段，然后再搬移函数。

做法

如果字段的访问级是public，使用Encapsulate Field (206)将它封装起来。

如果你有可能移动那些频繁访问该字段的函数，或如果有许多函数访问某个字段，先使用Self Encapsulate Field (171)也许会有帮助。

编译，测试。

在目标类中建立与源字段相同的字段，并同时建立相应的设值/取值函数。

编译目标类。

决定如何在源对象中引用目标对象。

首先看是否有一个现成的字段或函数可以助你得到目标对象。如果没有，就看能否轻易建立这样一个函数。如果还不行，就得在源类中新建一个字段来存放目标对象。这可能是个永久性修改，但你也可以让它是暂时的，因为后续重构可能会把这个新建字段除掉。

删除源字段。

将所有对源字段的引用替换为对某个目标函数的调用。

如果需要读取该变量，就把对源字段的引用替换为对目标取值函数的调用；如果要对该变量赋值，就把对源字段的引用替换成对设值函数的调用。

如果源字段不是private的，就必须在源类的所有子类中查找源字段的引用点，并进行相应替换。

编译，测试。

范例

下面是Account类的部分代码：

class Account...

private AccountType _type;

private double _interestRate;

double interestForAmount_days (double amount,int days){

return _interestRate * amount * days / 365;

}

我想把表示利率的_interestRate搬移到AccountType类去。目前已有数个函数引用了它，interestForAmount_days()就是其一。下一步我要在Account-Type中建立_interestRate字段以及相应的访问函数：

class AccountType...

private double _interestRate;

void setInterestRate (double arg){

_interestRate = arg;

}

double getInterestRate (){

return _interestRate;

}

这时候我可以编译新的AccountType类了。

现在，我需要让Account类中访问_interestRate字段的函数转而使用Account-Type对象，然后删除Account类中的_interestRate字段。我必须删除源字段，才能保证其访问函数的确改变了操作对象，因为编译器会帮我指出未被正确修改的函数。

private double _interestRate;

double interestForAmount_days (double amount,int days){

return _type.getInterestRate()* amount * days / 365;

}

范例：使用Self-Encapsulation

如果有很多函数已经使用了_interestRate字段，我应该先运用Self Encapsulate Field (171)（自我封装）：

class Account...

private AccountType _type;

private double _interestRate;

double interestForAmount_days(double amount,int days){

return getInterestRate()* amount * days / 365;

}

private void setInterestRate(double arg){

_interestRate = arg;

}

private double getInterestRate(){

return _interestRate;

}

这样，在搬移字段之后，我就只需要修改访问函数：

double interestForAmountAndDays(double amount,int days){

return getInterestRate()* amount * days / 365;

}

private void setInterestRate(double arg){

_type.setInterestRate(arg);

}

private double getInterestRate(){

return _type.getInterestRate();

}

以后若有必要，我可以修改访问函数的用户，让它们使用新对象。Self Encapsulate Field (171)使我得以保持小步前进。如果我需要对类做许多处理，保持小步前进是有帮助的。特别值得一提的是：首先使用Self Encapsulate Field (171)使我得以更轻松使用Move Method (142)将函数搬移到目标类中。如果待搬移函数引用了字段的访问函数，那些引用点是无需修改的。

---

## 7.3 Extract Class（提炼类）

某个类做了应该由两个类做的事。

建立一个新类，将相关的字段和函数从旧类搬移到新类。

动机

你也许听过类似这样的教诲：一个类应该是一个清楚的抽象，处理一些明确的责任。但是在实际工作中，类会不断成长扩展。你会在这儿加入一些功能，在那儿加入一些数据。给某个类添加一项新责任时，你会觉得不值得为这项责任分离出一个单独的类。于是，随着责任不断增加，这个类会变得过分复杂。很快，你的类就会变成一团乱麻。

这样的类往往含有大量函数和数据。这样的类往往太大而不易理解。此时你需要考虑哪些部分可以分离出去，并将它们分离到一个单独的类中。如果某些数据和某些函数总是一起出现，某些数据经常同时变化甚至彼此相依，这就表示你应该将它们分离出去。一个有用的测试就是问你自己，如果你搬移了某些字段和函数，会发生什么事？其他字段和函数是否因此变得无意义？

另一个往往在开发后期出现的信号是类的子类化方式。如果你发现子类化只影响类的部分特性，或如果你发现某些特性需要以一种方式来子类化，某些特性则需要以另一种方式子类化，这就意味你需要分解原来的类。

做法

决定如何分解类所负的责任。

建立一个新类，用以表现从旧类中分离出来的责任。

如果旧类剩下的责任与旧类名称不符，为旧类更名。

建立“从旧类访问新类”的连接关系。

有可能需要一个双向连接。但是在真正需要它之前，不要建立“从新类通往旧类”的连接。

对于你想搬移的每一个字段，运用Move Field (146)搬移之。

每次搬移后，编译、测试。

使用Move Method (142)将必要函数搬移到新类。先搬移较低层函数（也就是“被其他函数调用”多于“调用其他函数”者），再搬移较高层函数。

每次搬移之后，编译、测试。

检查，精简每个类的接口。

如果你建立起双向连接，检查是否可以将它改为单向连接。

决定是否公开新类。如果你的确需要公开它，就要决定让它成为引用对象还是不可变的值对象。

范例

我们从一个简单的Person类开始：

class Person...

public String getName(){

return _name;

}

public String getTelephoneNumber(){

return ("(" + _officeAreaCode + ")" + _officeNumber);

}

String getOfficeAreaCode(){

return _officeAreaCode;

}

void setOfficeAreaCode(String arg){

_officeAreaCode = arg;

}

String getOfficeNumber(){

return _officeNumber;

}

void setOfficeNumber(String arg){

_officeNumber = arg;

}

private String _name;

private String _officeAreaCode;

private String _officeNumber;

在这个例子中，我可以将与电话号码相关的行为分离到一个独立类中。首先我要定义一个TelephoneNumber类来表示“电话号码”这个概念：

class TelephoneNumber {

}

易如反掌！然后，我要建立从Person到TelephoneNumber的连接：

class Person

private TelephoneNumber _officeTelephone = new TelephoneNumber();

现在，我运用Move Field (146)移动一个字段：

class TelephoneNumber {

String getAreaCode(){

return _areaCode;

}

void setAreaCode(String arg){

_areaCode = arg;

}

private String _areaCode;

}

class Person...

public String getTelephoneNumber(){

return ("(" + getOfficeAreaCode()+ ")" + _officeNumber);

}

String getOfficeAreaCode(){

return _officeTelephone.getAreaCode();

}

void setOfficeAreaCode(String arg){

_officeTelephone.setAreaCode(arg);

}

然后我可以移动其他字段，并运用Move Method (142)将相关函数移动到Telep-honeNumber类中：

class Person...

public String getName(){

return _name;

}

public String getTelephoneNumber(){

return _officeTelephone.getTelephoneNumber();

}

TelephoneNumber getOfficeTelephone(){

return _officeTelephone;

}

private String _name;

private TelephoneNumber _officeTelephone = new TelephoneNumber();

class TelephoneNumber...

public String getTelephoneNumber(){

return ("(" + _areaCode + ")" + _number);

}

String getAreaCode(){

return _areaCode;

}

void setAreaCode(String arg){

_areaCode = arg;

}

String getNumber(){

return _number;

}

void setNumber(String arg){

_number = arg;

}

private String _number;

private String _areaCode;

下一步要做的决定是：要不要对用户公开这个新类？我可以将Person中与电话号码相关的函数委托至TelephoneNumber，从而完全隐藏这个新类；也可以直接将它对用户公开。我还可以将它公开给部分用户（位于同一个包中的用户），而不公开给其他用户。

如果我选择公开新类，就需要考虑别名带来的危险。如果我公开了Telephone-Number，而有个用户修改了对象中的_areaCode字段值，我又怎么能知道呢？而且，做出修改的可能不是直接用户，而是用户的用户的用户。

面对这个问题，我有下列几种选择。

1.允许任何对象修改TelephoneNumber对象的任何部分。这就使得Teleph-oneNumber对象成为引用对象，于是我应该考虑使用Change Value to Reference (179)。这种情况下，Person应该是TelephoneNumber的访问点。

2.不许任何人不通过Person对象就修改TelephoneNumber对象。为此，我可以将TelephoneNumber设为不可修改的，或为它提供一个不可修改的接口。

3.另一个办法是：先复制一个TelephoneNumber对象，然后将复制得到的新对象传递给用户。但这可能会造成一定程度的迷惑，因为人们会认为他们可以修改TelephoneNumber对象值。此外，如果同一个TelephoneNumber对象被传递给多个用户，也可能在用户之间造成别名问题。

Extract Class (149)是改善并发程序的一种常用技术，因为它使你可以为提炼后的两个类分别加锁。如果你不需要同时锁定两个对象，就不必这样做。这方面的更多信息请看Lea[Lea]的3.3节。

这里也存在危险性。如果需要确保两个对象被同时锁定，你就面临事务问题，需要使用其他类型的共享锁。正如Lea[Lea]的8.1节所讨论的，这是一个复杂领域，比起一般情况需要更繁重的机制。事务很有实用性，但是编写事务管理程序则超出了大多数程序员的职责范围。

---

## 7.4 Inline Class（将类内联化）

某个类没有做太多事情。

将这个类的所有特性搬移到另一个类中，然后移除原类。

动机

Inline Class (154)正好与Extract Class (149)相反。如果一个类不再承担足够责任、不再有单独存在的理由（这通常是因为此前的重构动作移走了这个类的责任），我就会挑选这一“萎缩类”的最频繁用户（也是个类），以Inline Class (154)手法将“萎缩类”塞进另一个类中。

做法

在目标类身上声明源类的public协议，并将其中所有函数委托至源类。

如果“以一个独立接口表示源类函数”更合适的话，就应该在内联之前先使用Extract Interface (341)。

修改所有源类引用点，改而引用目标类。

将源类声明为private，以斩断包之外的所有引用可能。同时修改源类的名称，这便可使编译器帮助你捕捉到所有对于源类的隐藏引用点。

编译，测试。

运用Move Method (142)和Move Field (146)，将源类的特性全部搬移到目标类。

为源类举行一个简单的“丧礼”。

范例

先前（上个重构项）我从TelephoneNumber提炼出另一个类，现在我要将它塞回到Person去。一开始这两个类是分离的：

class Person...

public String getName(){

return _name;

}

public String getTelephoneNumber(){

return _officeTelephone.getTelephoneNumber();

}

TelephoneNumber getOfficeTelephone(){

return _officeTelephone;

}

private String _name;

private TelephoneNumber _officeTelephone = new TelephoneNumber();

class TelephoneNumber...

public String getTelephoneNumber(){

return ("(" + _areaCode + ")" + _number);

}

String getAreaCode(){

return _areaCode;

}

void setAreaCode(String arg){

_areaCode = arg;

}

String getNumber(){

return _number;

}

void setNumber(String arg){

_number = arg;

}

private String _number;

private String _areaCode;

首先我在Person中声明TelephoneNumber的所有“可见”（public）函数：

class Person...

String getAreaCode(){

return _officeTelephone.getAreaCode();

}

void setAreaCode(String arg){

_officeTelephone.setAreaCode(arg);

}

String getNumber(){

return _officeTelephone.getNumber();

}

void setNumber(String arg){

_officeTelephone.setNumber(arg);

}

现在，我要找出TelephoneNumber的所有用户，让它们转而使用Person的接口。于是下列代码：

Person martin = new Person();

martin.getOfficeTelephone().setAreaCode ("781");

就变成了：

Person martin = new Person();

martin.setAreaCode ("781");

现在，我可以反复使用Move Method (142)和Move Field (146)，直到Telephone-Number不复存在。

---

## 7.5 Hide Delegate（隐藏“委托关系” ）

客户通过一个委托类来调用另一个对象。

在服务类上建立客户所需的所有函数，用以隐藏委托关系。

动机

“封装”即使不是对象的最关键特征，也是最关键特征之一。 “封装”意味每个对象都应该尽可能少了解系统的其他部分。如此一来，一旦发生变化，需要了解这一变化的对象就会比较少——这会使变化比较容易进行。

任何学过对象技术的人都知道：虽然Java允许将字段声明为public，但你还是应该隐藏对象的字段。随着经验日渐丰富，你会发现，有更多可以（而且值得）封装的东西。

如果某个客户先通过服务对象的字段得到另一个对象，然后调用后者的函数，那么客户就必须知晓这一层委托关系。万一委托关系发生变化，客户也得相应变化。你可以在服务对象上放置一个简单的委托函数，将委托关系隐藏起来，从而去除这种依赖（如图7-1所示）。这么一来，即便将来发生委托关系上的变化，变化也将被限制在服务对象中，不会波及客户。

图7-1 简单委托关系

对于某些或全部客户，你可能会发现，有必要先使用Extract Class (149)。一旦你对所有客户都隐藏了委托关系，就不再需要在服务对象的接口中公开被委托对象了。

做法

对于每一个委托关系中的函数，在服务对象端建立一个简单的委托函数。

调整客户，令它只调用服务对象提供的函数。

如果使用者和服务提供者不在同一个包，考虑修改委托函数的访问权限，让客户得以在包之外调用它。

每次调整后，编译并测试。

如果将来不再有任何客户需要取用图7-1所示的Delegate（受托类），便可移除服务对象中的相关访问函数。

编译，测试。

范例

本例从两个类开始：代表“人”的Person和代表“部门”的Department：

class Person {

Department _department;

public Department getDepartment(){

return _department;

}

public void setDepartment(Department arg){

_department = arg;

}

}

class Department {

private String _chargeCode;

private Person _manager;

public Department(Person manager){

_manager = manager;

}

public Person getManager(){

return _manager;

}

...

如果客户希望知道某人的经理是谁，他必须先取得Department对象：

manager = john.getDepartment().getManager();

这样的编码就是对客户揭露了Department的工作原理，于是客户知道：Depa-rtment用以追踪“经理”这条信息。如果对客户隐藏Department，可以减少耦合。为了这一目的，我在Person中建立一个简单的委托函数：

public Person getManager(){

return _department.getManager();

}

现在，我得修改Person的所有客户，让它们改用新函数：

manager = john.getManager();

只要完成了对Department所有函数的委托关系，并相应修改了Person的所有客户，我就可以移除Person中的访问函数getDepartment()了。

---

## 7.6 Remove Middle Man（移除中间人）

某个类做了过多的简单委托动作。

让客户直接调用受托类。

动机

在Hide Delegate (157)的“动机”一节中，我谈到了“封装受托对象”的好处。但是这层封装也是要付出代价的，它的代价就是：每当客户要使用受托类的新特性时，你就必须在服务端添加一个简单委托函数。随着受托类的特性（功能）越来越多，这一过程会让你痛苦不已。服务类完全变成了一个“中间人”，此时你就应该让客户直接调用受托类。

很难说什么程度的隐藏才是合适的。还好，有了Hide Delegate (157)和Remove Middle Man (160)，你大可不必操心这个问题，因为你可以在系统运行过程中不断进行调整。随着系统的变化，“合适的隐藏程度”这个尺度也相应改变。6个月前恰如其分的封装，现今可能就显得笨拙。重构的意义就在于：你永远不必说对不起——只要把出问题的地方修补好就行了。

做法

建立一个函数，用以获得受托对象。

对于每个委托函数，在服务类中删除该函数，并让需要调用该函数的客户转为调用受托对象。

处理每个委托函数后，编译、测试。

范例

我将以另一种方式使用先前用过的“人与部门”例子。还记得吗，上一项重构结束时，Person将Department隐藏起来了：

class Person...

Department _department;

public Person getManager(){

return _department.getManager();

class Department...

private Person _manager;

public Department (Person manager){

_manager = manager;

}

为了找出某人的经理，客户代码可能这样写：

manager = john.getManager();

像这样，使用和封装Department都很简单。但如果大量函数都这么做，我就不得不在Person之中安置大量委托行为。这就该是移除中间人的时候了。首先在Person中建立一个函数用于获得受托对象：

class Person...

public Department getDepartment(){

return _department;

}

然后逐一处理每个委托函数。针对每一个这样的函数，我要找出通过Person使用的函数，并对它进行修改，使它首先获得受托对象，然后直接使用后者：

manager = john.getDepartment().getManager();

然后我就可以删除Person的getManager()函数。如果我遗漏了什么，编译器会告诉我。

为方便起见，我也可能想要保留一部分委托关系。此外，我也可能希望对某些客户隐藏委托关系，并让另一些用户直接使用受托对象。基于这些原因，一些简单的委托关系（以及对应的委托函数）也可能被留在原地。

---

## 7.7 Introduce Foreign Method（引入外加函数）

你需要为提供服务的类增加一个函数，但你无法修改这个类。

在客户类中建立一个函数，并以第一参数形式传入一个服务类实例。

Date newStart = new Date (previousEnd.getYear(),
previousEnd.getMonth(),previousEnd.getDate()+ 1);

Date newStart = nextDay(previousEnd);
private static Date nextDay(Date arg){
return new Date (arg.getYear(),arg.getMonth(),arg.getDate()+ 1);
}

动机

这种事情发生过太多次了：你正在使用一个类，它真的很好，为你提供了需要的所有服务。而后，你又需要一项新服务，这个类却无法供应。于是你开始咒骂：“为什么不能做这件事?”如果可以修改源码，你便可以自行添加一个新函数；如果不能，你就得在客户端编码，补足你要的那个函数。

如果客户类只使用这项功能一次，那么额外编码工作没什么大不了，甚至可能根本不需要原本提供服务的那个类。然而，如果你需要多次使用这个函数，就得不断重复这些代码。还记得吗，重复代码是软件万恶之源。这些重复代码应该被抽出来放进同一个函数中。进行本项重构时，如果你以外加函数实现一项功能，那就是一个明确信号：这个函数原本应该在提供服务的类中实现。

如果你发现自己为一个服务类建立了大量外加函数，或者发现有许多类都需要同样的外加函数，就不应该再使用本项重构，而应该使用Introduce Local Extension (164)。

但是不要忘记：外加函数终归是权宜之计。如果有可能，你仍然应该将这些函数搬移到它们的理想家园。如果由于代码所有权的原因使你无法做这样的搬移，就把外加函数交给服务类的拥有者，请他帮你在服务类中实现这个函数。

做法

在客户类中建立一个函数，用来提供你需要的功能。

这个函数不应该调用客户类的任何特性。如果它需要一个值，把该值当作参数传给它。

以服务类实例作为该函数的第一个参数。

将该函数注释为：“外加函数（foreign method），应在服务类实现。”

这么一来，如果将来有机会将外加函数搬移到服务类中时，你便可以轻松找出这些外加函数。

范例

程序中，我需要跨过一个收费周期。原本代码像这样：

Date newStart = new Date (previousEnd.getYear(),

previousEnd.getMonth(),previousEnd.getDate()+ 1);

我可以将赋值运算右侧代码提炼到一个独立函数中。这个函数就是Date类的一个外加函数：

Date newStart = nextDay(previousEnd);

private static Date nextDay(Date arg){

// foreign method,should be on date

return new Date (arg.getYear(),arg.getMonth(),arg.getDate()+ 1);

}

---

## 7.8 Introduce Local Extension（引入本地扩展）

你需要为服务类提供一些额外函数，但你无法修改这个类。

建立一个新类，使它包含这些额外函数。让这个扩展品成为源类的子类或包装类。

动机

很遗憾，类的作者无法预知未来，他们常常没能为你预先准备一些有用的函数。如果你可以修改源码，最好的办法就是直接加入自己需要的函数。但你经常无法修改源码。如果只需要一两个函数，你可以使用Introduce Foreign Method (162)。但如果你需要的额外函数超过两个，外加函数就很难控制它们了。所以，你需要将这些函数组织在一起，放到一个恰当地方去。要达到这一目的，两种标准对象技术——子类化（subclassing）和包装（wrapping）——是显而易见的办法。这种情况下，我把子类或包装类统称为本地扩展（local extension）。

所谓本地扩展是一个独立的类，但也是被扩展类的子类型：它提供源类的一切特性，同时额外添加新特性。在任何使用源类的地方，你都可以使用本地扩展取而代之。

使用本地扩展使你得以坚持“函数和数据应该被统一封装”的原则。如果你一直把本该放在扩展类中的代码零散地放置于其他类中，最终只会让其他这些类变得过分复杂，并使得其中函数难以被复用。

在子类和包装类之间做选择时，我通常首选子类，因为这样的工作量比较少。制作子类的最大障碍在于，它必须在对象创建期实施。如果我可以接管对象创建过程，那当然没问题；但如果你想在对象创建之后再使用本地扩展，就有问题了。此外，子类化方案还必须产生一个子类对象，这种情况下，如果有其他对象引用了旧对象，我们就同时有两个对象保存了原数据！如果原数据是不可修改的，那也没问题，我可以放心进行复制；但如果原数据允许被修改，问题就来了，因为一个修改动作无法同时改变两份副本。这时候我就必须改用包装类。使用包装类时，对本地扩展的修改会波及原对象，反之亦然。

做法

建立一个扩展类，将它作为原始类的子类或包装类。

在扩展类中加入转型构造函数。

所谓“转型构造函数”是指“接受原对象作为参数”的构造函数。如果采用子类化方案，那么转型构造函数应该调用适当的超类构造函数；如果采用包装类方案，那么转型构造函数应该将它得到的传入参数以实例变量的形式保存起来，用作接受委托的原对象。

在扩展类中加入新特性。

根据需要，将原对象替换为扩展对象。

将针对原始类定义的所有外加函数搬移到扩展类中。

范例

我将以Java l.0.1的Date类为例。Java l.1已经提供了我想要的功能，但是在它到来之前的那段日子，很多时候我需要扩展Java l.0.1的Date类。

第一件待决事项就是：使用子类还是包装类。子类化是比较显而易见的办法：

class MfDateSub extends Date {

public MfDateSub nextDay()...

public int dayOfYear()...

包装类则需用上委托：

class MfDateWrap {

private Date _original;

范例：使用子类

首先，我要新建立一个MfDateSub
[1]

 类来表示“日期”，并使其成为Date的子类：

class MfDateSub extends Date

然后，我需要处理Date和扩展类之间的不同处。MfDateSub构造函数需要委托给Date构造函数：

public MfDateSub (String dateString){

super (dateString);

};

现在，我需要加入一个转型构造函数，其参数是一个源类的对象：

public MfDateSub (Date arg){

super (arg.getTime());

}

现在，我可以在扩展类中添加新特性，并使用Move Method (142)将所有外加函数搬移到扩展类。于是，下面的代码：

client class...

private static Date nextDay(Date arg){

// foreign method,should be on date

return new Date (arg.getYear(),arg.getMonth(),arg.getDate()+ 1);

}

经过搬移之后，就成了：

class MfDateSub...

Date nextDay(){

return new Date (getYear(),getMonth(),getDate()+ 1);

}

范例：使用包装类

首先声明一个包装类：

class MfDateWrap {

private Date _original;

}

使用包装类方案时，我对构造函数的设定与先前有所不同。现在的构造函数将只执行一个单纯的委托动作：

public MfDateWrap (String dateString){

_original = new Date(dateString);

};

而转型构造函数则只是对其实例变量赋值而已：

public MfDateWrap (Date arg){

_original = arg;

}

接下来是一项枯燥乏味的工作：为原始类的所有函数提供委托函数。我只展示两个函数，其他函数的处理依此类推。

public int getYear(){

return _original.getYear();

}

public boolean equals(Object arg){

if (this == arg)

return true;

if (!(arg instanceof MfDateWrap))

return false;

MfDateWrap other = ((MfDateWrap)arg);

return (_original.equals(other._original));

}

完成这项工作之后，我就可以后使用Move Method (142)将日期相关行为搬移到新类中。于是以下代码：

client class...

private static Date nextDay(Date arg){

// foreign method,should be on date

return new Date (arg.getYear(),arg.getMonth(),arg.getDate()+ 1);

}

经过搬移之后，就成了：

class MfDateWrap...

Date nextDay(){

return new Date (getYear(),getMonth(),getDate()+ 1);

}

使用包装类有一个特殊问题：如何处理“接受原始类之实例为参数”的函数？例如：

public boolean after (Date arg)

由于无法改变原始类，所以我只能做到在一个方向上的兼容——包装类上的after()函数可以接受包装类或原始类的对象；但原始类的after()函数只能接受原始类对象，不接受包装类对象：

aWrapper.after(aDate)　　　　　　// can be made to work

aWrapper.after(anotherWrapper)// can be made to work

aDate.after(aWrapper)　　　　　　// will not work

这样覆写的目的是为了向用户隐藏包装类的存在。这是一个好策略，因为包装类的用户的确不应该关心包装类的存在，的确应该可以同样地对待包装类和原始类。但是我无法完全隐藏包装类的存在，因为某些系统所提供的函数（例如equals()）会出问题。你可能会认为：你可以在MfDatewrap类中覆写equals()，像这样：

public boolean equals (Date arg)　　// causes problems

但这样做是危险的，因为尽管我达到了自己的目的，但Java系统的其他部分都认为equals()符合交换律：如果a.equals(b)为真，那么b.equals(a)也必为真。违反这一规则将使我遭遇一大堆莫名其妙的错误。要避免这样的尴尬境地，唯一的办法就是修改Date类。但如果我能够修改Date，又何必进行此项重构？所以，在这种情况下，我只能向用户公开“我进行了包装”这一事实。我将以一个新函数来进行日期之间的相等性检查：

public boolean equalsDate (Date arg)

我可以重载equalsDate()，让一个重载版本接受Date对象，另一个重载版本接受MfDatewrap对象。这样我就不必检查未知对象的类型了：

public boolean equalsDate (MfDateWrap arg)

子类化方案中就没有这样的问题，只要我不覆写原函数就行了。但如果我覆写了原始类中的函数，那么寻找函数时，就会被搞得晕头转向。一般来说，我不会在扩展类中覆写原始类的函数，只会添加新函数。

[1].
 Mf是作者Martin Fowler的姓名缩写。——译者注

---


# 第8章 重新组织数据 (Organizing Data)

## 8.1 Self Encapsulate Field（自封装字段）

你直接访问一个字段，但与字段之间的耦合关系逐渐变得笨拙。

为这个字段建立取值/设值函数，并且只以这些函数来访问字段。

private int _low,_high;
boolean includes (int arg){
return arg >= _low && arg <= _high;
}

private int _low,_high;
boolean includes (int arg){
return arg >= getLow()&& arg <= getHigh();
}
int getLow(){return _low;}
int getHigh(){return _high;}

动机

在“字段访问方式”这个问题上，存在两种截然不同的观点：其中一派认为，在该变量定义所在的类中，你可以自由访问它；另一派认为，即使在这个类中你也应该只使用访问函数间接访问。两派之间的争论可以说是如火如荼。（参见Auer在[Auer]p.413和Beck在[Beck]上的讨论。）

归根结底，间接访问变量的好处是，子类可以通过覆写一个函数而改变获取数据的途径；它还支持更灵活的数据管理方式，例如延迟初始化（意思是：只有在需要用到某值时，才对它初始化）。

直接访问变量的好处则是：代码比较容易阅读。阅读代码的时候，你不需要停下来说：“啊，这只是个取值函数。”

面临选择时，我总是做两手准备。通常情况下我会很乐意按照团队中其他人的意愿来做。就我自己而言，我比较喜欢先使用直接访问方式，直到这种方式给我带来麻烦为止，此时我就会转而使用间接访问方式。重构给了我改变主意的自由。

如果你想访问超类中的一个字段，却又想在子类中将对这个变量的访问改为一个计算后的值，这就是最该使用Self Encapsulate Field (171)的时候。“字段自我封装”只是第一步。完成自我封装之后，你可以在子类中根据自己的需要随意覆写取值/设值函数。

做法

为待封装字段建立取值/设值函数。

找出该字段的所有引用点，将它们全部改为调用取值/设值函数。

如果引用点要读取字段值，就将它替换为调用取值函数；如果引用点要给字段赋值，就将它替换为调用设值函数。

你可以暂时将该字段改名，让编译器帮助你查找引用点。

将该字段声明为private。

复查，确保找出所有引用点。

编译，测试。

范例

下面这个例子看上去有点过分简单。不过，嘿，起码它写起来很快：

class IntRange {

private int _low,_high;

boolean includes(int arg){

return arg >= _low && arg <= _high;

}

void grow(int factor){

_high = _high * factor;

}

IntRange(int low,int high){

_low = low;

_high = high;

}

为了封装_low和_high这两个字段，我先定义取值/设值函数（如果此前没有定义的话），并使用它们：

class IntRange {

boolean includes(int arg){

return arg >= getLow()&& arg <= getHigh();

}

void grow(int factor){

setHigh(getHigh()* factor);

}

private int _low,_high;

int getLow(){

return _low;

}

int getHigh(){

return _high;

}

void setLow(int arg){

_low = arg;

}

void setHigh(int arg){

_high = arg;

}

}

使用本项重构时，你必须小心对待“在构造函数中使用设值函数”的情况。一般说来，设值函数被认为应该在对象创建后才使用，所以初始化过程中的行为有可能与设值函数的行为不同。这种情况下，我也许在构造函数中直接访问字段，要不就是单独另建一个初始化函数：

IntRange(int low,int high){

initialize(low,high);

}

private void initialize(int low,int high){

_low = low;

_high = high;

}

一旦你拥有一个子类，上述所有动作的价值就体现出来了。如下所示：

class CappedRange extends IntRange {

CappedRange(int low,int high,int cap){

super(low,high);

_cap = cap;

}

private int _cap;

int getCap(){

return _cap;

}

int getHigh(){

return Math.min(super.getHigh(),getCap());

}

}

现在，我可以在CappedRange中覆写getHigh()，从而加入对“范围上限”（cap）的考虑，而不必修改IntRange的任何行为。

---

## 8.2 Replace Data Value with Object（以对象取代数据值）

你有一个数据项，需要与其他数据和行为一起使用才有意义。

将数据项变成对象。

动机

开发初期，你往往决定以简单的数据项表示简单的情况。但是，随着开发的进行，你可能会发现，这些简单数据项不再那么简单了。比如说，一开始你可能会用一个字符串来表示“电话号码”概念，但是随后你就会发现，电话号码需要“格式化”、“抽取区号”之类的特殊行为。如果这样的数据项只有一两个，你还可以把相关函数放进数据项所属的对象里；但是Duplicate Code坏味道和Feature Envy坏味道很快就会从代码中散发出来。当这些坏味道开始出现，你就应该将数据值变成对象。

做法

为待替换数值新建一个类，在其中声明一个final字段，其类型和源类中的待替换数值类型一样。然后在新类中加入这个字段的取值函数，再加上一个接受此字段为参数的构造函数。

编译。

将源类中的待替换数值字段的类型改为前面新建的类。

修改源类中该字段的取值函数，令它调用新类的取值函数。

如果源类构造函数中用到这个待替换字段（多半是赋值动作），我们就修改构造函数，令它改用新类的构造函数来对字段进行赋值动作。

修改源类中待替换字段的设值函数，令它为新类创建一个实例。

编译，测试。

现在，你有可能需要对新类使用Change Value to Reference (179)。

范例

下面有一个代表“定单”的Order类，其中以一个字符串记录定单客户。现在，我希望改用一个对象来表示客户信息，这样就有充裕的弹性保存客户地址、信用等级等信息，也得以安置这些信息的操作行为。Order类最初如下：

class Order...

public Order(String customer){

_customer = customer;

}

public String getCustomer(){

return _customer;

}

public void setCustomer(String arg){

_customer = arg;

}

private String _customer;

使用Order类的代码可能像下面这样：

private static int numberOfOrdersFor(Collection orders,String customer){

int result = 0;

Iterator iter = orders.iterator();

while (iter.hasNext()){

Order each = (Order)iter.next();

if (each.getCustomer().equals(customer))result++;

}

return result;

}

首先，我要新建一个Customer类来表示“客户”概念。然后在这个类中建立一个final字段，用以保存一个字符串，这是Order类目前所使用的。我将这个新字段命名为_name，因为这个字符串的用途就是记录客户名称。此外我还要为这个字符串加上取值函数和构造函数。

class Customer {

public Customer(String name){

_name = name;

}

ublic String getName(){

return _name;

}

private final String _name;

}

现在，我要将Order中的_customer字段的类型修改为Customer；并修改所有引用该字段的函数，让它们恰当地改而引用Customer对象。其中取值函数和构造函数的修改都很简单。至于设值函数，我让它创建一个customer实例。

class Order...

public Order(String customer){

_customer = new Customer(customer);

}

public String getCustomer(){

return _customer.getName();

}

private Customer _customer;

public void setCustomer(String arg){

_customer = new Customer(arg);

}

设值函数需要创建一个Customer实例，这是因为以前的字符串是个值对象（value object），所以现在的Customer对象也应该是个值对象。这也就意味每个Order对象都包含自己的一个Customer对象。注意这样一条规则：值对象应该是不可修改内容的——这便可以避免一些讨厌的别名问题。日后或许我会想让Customer对象成为引用对象（reference object），但那是另一项重构手法的责任。现在我可以编译并测试了。

我需要观察Order类中的_customer字段的操作函数，并作出一些修改，使它更好地反映出修改后的新形势。对于取值函数，我会使用Rename Method (273)改变其名称，让它更清晰地表示，它所返回的是消费者名称，而不是个Customer对象。

public String getCustomerName(){

return _customer.getName();

}

至于构造函数和设值函数，我就不必修改其签名了，但参数名称得改：

public Order (String customerName){

_customer = new Customer(customerName);

}

public void setCustomer(String customerName){

_customer = new Customer(customerName);

}

后继的其他重构也许会添加“接受现有Customer对象作为参数”的构造函数和设值函数。

本次重构到此为止。但是，这个案例和其他很多案例一样，还需要一个后续步骤。如果想在Customer中加入信用等级、地址之类的其他信息，现在还做不到，因为目前的Customer还是作为值对象对待的，每个Order对象都拥有自己的Customer对象。为了给Customer类加上信用等级、地址之类的属性，我必须运用Change Value to Reference (179)，这么一来，属于同一客户的所有Order对象就可以共享同一个Customer对象了。马上你就可以看到这个例子。

---

## 8.3 Change Value to Reference（将值对象改为引用对象）

你从一个类衍生出许多彼此相等的实例，希望将它们替换为同一个对象。

将这个值对象变成引用对象。

动机

在许多系统中，你都可以对对象做一个有用的分类：引用对象和值对象。前者就像“客户”、“账户”这样的东西，每个对象都代表真实世界中的一个实物，你可以直接以相等操作符（==，用来检验对象同一性）检查两个对象是否相等。后者则是像“日期”、“钱”这样的东西，它们完全由其所含的数据值来定义，你并不在意副本的存在，系统中或许存在成百上千个内容为“1/1/2000”的“日期”对象。当然，你也需要知道两个值对象是否相等，所以你需要覆写equals()（以及hashCode()）。

要在引用对象和值对象之间做选择有时并不容易。有时候，你会从一个简单的值对象开始，在其中保存少量不可修改的数据。而后，你可能会希望给这个对象加入一些可修改数据，并确保对任何一个对象的修改都能影响到所有引用此一对象的地方。这时候你就需要将这个对象变成一个引用对象。

做法

使用Replace Constructor with Factory Method (304)。

编译，测试。

决定由什么对象负责提供访问新对象的途径。

可能是一个静态字典或一个注册表对象。

你也可以使用多个对象作为新对象的访问点。

决定这些引用对象应该预先创建好，或是应该动态创建。

如果这些引用对象是预先创建好的，而你必须从内存中将它们读取出来，那么就得确保它们在被需要的时候能够被及时加载。

修改工厂函数，令它返回引用对象。

如果对象是预先创建好的，你就需要考虑：万一有人索求一个其实并不存在的对象，要如何处理错误？

你可能希望对工厂函数使用Rename Method (273)，使其传达这样的信息：它返回的是一个既存对象。

编译，测试。

范例

在Replace Data Value with Object (175)一节中，我留下了一个重构后的程序，本节范例就从它开始。我们有下列的Customer类:

class Customer {

public Customer(String name){

_name = name;

}

public String getName(){

return _name;

}

private final String _name;

}

它被以下的Order类使用：

class Order...

public Order(String customerName){

_customer = new Customer(customerName);

}

public void setCustomer(String customerName){

_customer = new Customer(customerName);

}

public String getCustomerName(){

return _customer.getName();

}

private Customer _customer;

此外，还有一些代码也会使用Customer对象：

private static int numberOfOrdersFor(Collection orders,String customer){

int result = 0;

Iterator iter = orders.iterator();

while (iter.hasNext()){

Order each = (Order)iter.next();

if (each.getCustomerName().equals(customer))

result++;

}

return result;

}

到目前为止，Customer对象还是值对象。就算多份定单属于同一客户，但每个Order对象还是拥有各自的Customer对象。我希望改变这一现状，使得一旦同一客户拥有多份不同定单，代表这些定单的所有Order对象就可以共享同一个Customer对象。本例中，这就意味着：每一个客户名称只该对应一个Customer对象。

首先我使用Replace Constructor with Factory Method (304)。这样，我就可以控制Customer对象的创建过程，这在以后会是非常重要的。我在Customer类中定义这个工厂函数：

class Customer {

public static Customer create (String name){

return new Customer(name);

}

然后把原本调用构造函数的地方改为调用工厂函数：

class Order {

public Order (String customer){

_customer = Customer.create(customer);

}

然后再把构造函数声明为private：

class Customer {

private Customer (String name){

_name = name;

}

现在，我必须决定如何访问Customer对象。我比较喜欢通过另一个对象（例如Order中的一个字段）来访问它。但是本例并没有这样一个明显的字段可用于访问Customer对象。在这种情况下，我通常会创建一个注册表对象来保存所有Customer对象，以此作为访问点。为了简化我们的例子，我把这个注册表保存在Customer类的static字段中，让Customer类作为访问点：

private static Dictionary _instances = new Hashtable();

然后我得决定：应该在接到请求时创建新的Customer对象，还是应该预先将它们创建好。这里我选择后者。在应用程序的启动代码中，我先把需要使用的Customer对象加载妥当。这些对象可能来自数据库，也可能来自文件。为求简单起见，我在代码中明确生成这些对象。反正以后我总是可以使用Substitute Algorithm (139)来改变它们的创建方式。

class Customer...

static void loadCustomers(){

new Customer("Lemon Car Hire").store();

new Customer("Associated Coffee Machines").store();

new Customer("Bilston Gasworks").store();

}

private void store(){

_instances.put(this.getName(),this);

}

现在，我要修改工厂函数，让它返回预先创建好的Customer对象：

public static Customer create (String name){
    return (Customer)_instances.get(name);
}

由于create()总是返回既有的Customer对象，所以我应该使用Rename Method (273)修改这个工厂函数的名称，以便强调这一点。

class Customer...

public static Customer getNamed (String name){

return (Customer)_instances.get(name);

}

---

## 8.4 Change Reference to Value（将引用对象改为值对象）

你有一个引用对象，很小且不可变，而且不易管理。

将它变成一个值对象。

动机

正如我在Change Value to Reference (179)中所说，要在引用对象和值对象之间做选择，有时并不容易。作出选择后，你常会需要一条回头路。

如果引用对象开始变得难以使用，也许就应该将它改为值对象。引用对象必须被某种方式控制，你总是必须向其控制者请求适当的引用对象。它们可能造成内存区域之间错综复杂的关联。在分布系统和并发系统中，不可变的值对象特别有用，因为你无需考虑它们的同步问题。

值对象有一个非常重要的特性：它们应该是不可变的。无论何时，只要你调用同一对象的同一个查询函数，都应该得到同样结果。如果保证了这一点，就可以放心地以多个对象表示同一个事物。如果值对象是可变的，你就必须确保对某一对象的修改会自动更新其他“代表相同事物”的对象。这太痛苦了，与其如此还不如把它变成引用对象。

这里有必要澄清一下“不可变”（immutable）的意思。如果你以Money类表示“钱”的概念，其中有“币种”和“金额”两条信息，那么Money对象通常是一个不可变的值对象。这并非意味你的薪资不能改变，而是意味：如果要改变你的薪资，就需要使用另一个Money对象来取代现有的Money对象，而不是在现有的Money对象上修改。你和Money对象之间的关系可以改变，但Money对象自身不能改变。

做法

检查重构目标是否为不可变对象，或是否可修改为不可变对象。

如果该对象目前还不是不可变的，就使用Remove Setting Method (300)，直到它成为不可变的为止。

如果无法将该对象修改为不可变的，就放弃使用本项重构。

建立equals()和hashCode()。

编译，测试。

考虑是否可以删除工厂函数，并将构造函数声明为public。

范例

我们从一个表示“货币种类”的Currency类开始：

class Currency...

private String _code;

public String getCode(){

return _code;

}

private Currency(String code){

_code = code;

}

这个类所做的就是保存并返回一个货币种类代码。它是一个引用对象，所以如果要得到它的实例，必须这么做：

Currency usd = Currency.get("USD");

Currency类维护一个包含所有Currency实例的链表。我不能直接使用构造函数创建实例，因为Currency构造函数是private的。

new Currency("USD").equals(new Currency("USD"))// returns false

要把一个引用对象变成值对象，关键动作是：检查它是否不可变。如果不是，我就不能使用本项重构，因为可变的值对象会造成烦人的别名问题。

在这里，Currency对象是不可变的，所以下一步就是为它定义equals()：

public boolean equals(Object arg){

if (! (arg instanceof Currency))return false;

Currency other = (Currency)arg;

return (_code.equals(other._code));

}

定义了equals()，就必须同时定义hashCode()。实现hashCode()有个简单办法：读取equals()使用的所有字段的hash码，然后对它们进行按位异或（^）操作。本例中，这很容易实现，因为equals()只使用了一个字段：

public int hashCode(){

return _code.hashCode();

}

完成这两个函数后，我可以编译并测试。这两个函数的修改必须同时进行，否则倚赖hash的任何集合对象（例如Hashtable、HashSet和HashMap）都可能会产生意外行为。

现在，我想创建多少个等值的Currency对象就可以创建多少个。我还可以把构造函数声明为public，直接以构造函数获取Currency实例，从而去掉Currency类中的工厂函数和控制实例创建的行为。

new Currency("USD").equals(new Currency("USD"))// now returns true

---

## 8.5 Replace Array with Object（以对象取代数组）

你有一个数组，其中的元素各自代表不同的东西。

以对象替换数组。对于数组中的每个元素，以一个字段来表示。

String[] row = new String[3];
row [0] = "Liverpool";
row [1] = "15";

Performance row = new Performance();
row.setName("Liverpool");
row.setWins("15");

动机

数组是一种常见的用以组织数据的结构。不过，它们应该只用于“以某种顺序容纳一组相似对象”。有时候你会发现，一个数组容纳了多种不同对象，这会给用户带来麻烦，因为他们很难记住像“数组的第一个元素是人名”这样的约定。对象就不同了，你可以运用字段名称和函数名称来传达这样的信息，因此你无需死记它，也无需依赖注释。而且如果使用对象，你还可以将信息封装起来，并使用Move Method (142)为它加上相关行为。

做法

新建一个类表示数组所拥有的信息，并在其中以一个public字段保存原先的数组。

修改数组的所有用户，让它们改用新类的实例。

编译，测试。

逐一为数组元素添加取值/设值函数。根据元素的用途，为这些访问函数命名。修改客户端代码，让它们通过访问函数取用数组内的元素。每次修改后，编译并测试。

当所有对数组的直接访问都转而调用访问函数后，将新类中保存该数组的字段声明为private。

编译。

对于数组内的每一个元素，在新类中创建一个类型相当的字段。修改该元素的访问函数，令它改用上述的新建字段。

每修改一个元素，编译并测试。

数组的所有元素都有了相应字段之后，删除该数组。

范例

我们的范例从一个数组开始，其中有3个元素，分别保存一支球队的名称、获胜场次和失利场次。这个数组的声明可能像这样：

String[] row = new String[3];

而使用它的代码则可能像这样：

row [0] = "Liverpool";

row [1] = "15";

String name = row[0];

int wins = Integer.parseInt(row[1]);

为了将数组变成对象，我首先建立一个对应的类:

class Performance {}

然后为它声明一个public字段，用以保存原先数组。（我知道public字段十恶不赦，请放心，稍后我便让它改邪归正。）

public String[] _data = new String[3];

现在，我要找到创建和访问数组的地方。在创建地点，我将它替换为下列代码：

Performance row = new Performance();

对于数组使用地点，我将它替换为以下代码：

row._data [0] = "Liverpool";

row._data [1] = "15";

String name = row._data[0];

int wins = Integer.parseInt(row._data[1]);

然后我要逐一为数组元素加上有意义的取值/设值函数。首先从“球队名称”开始：

class Performance...

public String getName(){

return _data[0];

}

public void setName(String arg){

_data[0] = arg;

}

然后修改使用row对象的代码，让它们改用这些函数来访问球队名称：

row.setName("Liverpool");

row._data [1] = "15";

String name = row.getName();

int wins = Integer.parseInt(row._data[1]);

第二个元素也如法炮制。为了简单起见，我还可以把数据类型的转换也封装起来：

class Performance...

public int getWins(){

return Integer.parseInt(_data[1]);

}

public void setWins(String arg){

_data[1] = arg;

}

....

client code...

row.setName("Liverpool");

row.setWins("15");

String name = row.getName();

int wins = row.getWins();

处理完所有元素之后，我就可以将保存该数组的字段声明为private了。

private String[] _data = new String[3];

现在，本次重构最重要的部分（接口修改）已经完成。但是“将对象内的数组替换掉”的过程也同样重要。我可以针对每个数组元素，在Performance类建立一个类型相当的字段，然后修改该数组元素的访问函数，令它直接访问新建字段，从而完全摆脱对数组元素的依赖。

class Performance...

public String getName(){

return _name;

}

public void setName(String arg){

_name = arg;

}

private String _name;

对数组中的每一个元素都如法炮制。全部处理完毕后，我就可以将数组从Performance类中删掉了。

---

## 8.6 Duplicate Observed Data（复制“被监视数据”）

你有一些领域数据置身于GUI控件中，而领域函数需要访问这些数据。

将该数据复制到一个领域对象中。建立一个Observer模式，用以同步领域对象和GUI对象内的重复数据。

动机

一个分层良好的系统，应该将处理用户界面和处理业务逻辑的代码分开。之所以这样做，原因有以下几点：(1)你可能需要使用不同的用户界面来表现相同的业务逻辑，如果同时承担两种责任，用户界面会变得过分复杂；(2)与GUI隔离之后，领域对象的维护和演化都会更容易，你甚至可以让不同的开发者负责不同部分的开发。

尽管可以轻松地将“行为”划分到不同部位，“数据”却往往不能如此。同一项数据有可能既需要内嵌于GUI控件，也需要保存于领域模型里。自从MVC （Model-View-Controller，模型-视图-控制器）模式出现后，用户界面框架都使用多层系统来提供某种机制，使你不但可以提供这类数据，并保持它们同步。

如果你遇到的代码是以两层方式开发，业务逻辑被内嵌于用户界面之中，你就有必要将行为分离出来。其中的主要工作就是函数的分解和搬移。但数据就不同了：你不能仅仅只是移动数据，必须将它复制到新的对象中，并提供相应的同步机制。

做法
[1]

修改展现类，使其成为领域类的Observer[GoF]。

如果尚未有领域类，就建立一个。

如果没有“从展现类到领域类”的关联，就将领域类保存于展现类的一个字段中。

针对GUI类中的领域数据，使用Self Encapsulate Field (171)。

编译，测试。

在事件处理函数中调用设值函数，直接更新GUI组件。

在事件处理函数中放一个设值函数，利用它将GUI组件更新为领域数据的当前值。当然这其实没有必要，你只不过是拿它的值设定它自己。但是这样使用设值函数，便是允许其中的任何动作得以于日后被执行起来，这是这一步骤的意义所在。

进行这个改变时，对于组件，不要使用取值函数，应该直接取用，因为稍后我们将修改取值函数，使其从领域对象（而非GUI组件）取值。设值函数也将做类似修改。

确保测试代码能够触发新添加的事件处理机制。

编译，测试。

在领域类中定义数据及其相关访问函数。

确保领域类中的设值函数能够触发Observer模式的通报机制。

对于被观察的数据，在领域类中使用与展现类所用的相同类型（通常是字符串）来保存。后续重构中你可以自由改变这个数据类型。

修改展现类中的访问函数，将它们的操作对象改为领域对象（而非GUI组件）。

修改Observer的update()，使其从相应的领域对象中将所需数据复制给GUI组件。

编译，测试。

范例

我们的范例从图8-1所示窗口开始。其行为非常简单：当用户修改文本框中的数值，另两个文本框就会自动更新。如果你修改Start或End，Length就会自动成为两者计算所得的长度；如果你修改Length，End就会随之变动。

图8-1 一个简单的GUI窗口

一开始，所有函数都放在IntervalWindow类中。所有文本框都能够响应“失去焦点”这一事件。

public class IntervalWindow extends Frame...

java.awt.TextField _startField;

java.awt.TextField _endField;

java.awt.TextField _lengthField;

class SymFocus extends java.awt.event.FocusAdapter

{

public void focusLost(java.awt.event.FocusEvent event)

{

Object object = event.getSource();

if (object == _startField)

StartField_FocusLost(event);

else if (object == _endField)

EndField_FocusLost(event);

else if (object == _lengthField)

LengthField_FocusLost(event);

}

}

当Start文本框失去焦点，事件监听器调用StartField_FocusLost()。另两个文本框的处理也类似。事件处理函数大致如下：

void StartField_FocusLost(java.awt.event.FocusEvent event){

if (isNotInteger(_startField.getText()))

_startField.setText("0");

calculateLength();

}

void EndField_FocusLost(java.awt.event.FocusEvent event){

if (isNotInteger(_endField.getText()))

_endField.setText("0");

calculateLength();

}

void LengthField_FocusLost(java.awt.event.FocusEvent event){

if (isNotInteger(_lengthField.getText()))

_lengthField.setText("0");

calculateEnd();

}

你也许会奇怪，为什么我这样实现一个窗口呢？因为在我的集成开发环境Cafe中，这是最简单的方式。

如果文本框内的字符串无法转换为一个整数，那么该文本框的内容将变成0。而后，调用相关计算函数：

void calculateLength(){

try {

int start = Integer.parseInt(_startField.getText());

int end = Integer.parseInt(_endField.getText());

int length = end - start;

_lengthField.setText(String.valueOf(length));

} catch (NumberFormatException e){

throw new RuntimeException("Unexpected Number Format Error");

}

}

void calculateEnd(){

try {

int start = Integer.parseInt(_startField.getText());

int length = Integer.parseInt(_lengthField.getText());

int end = start + length;

_endField.setText(String.valueOf(end));

} catch (NumberFormatException e){

throw new RuntimeException("Unexpected Number Format Error");

}

}

我的任务就是将与展现无关的计算逻辑从GUI中分离出来。基本上这就意味将calculateLength()和calculateEnd()移到一个独立的领域类去。为了这一目的，我需要能够在不引用窗口类的前提下获取Start、End和Length三个文本框的值。唯一办法就是将这些数据复制到领域类中，并保持与GUI类数据同步。这就是Duplicate Observed Data (189)的任务。

截至目前我还没有一个领域类，所以要着手建立一个（空的）：

class Interval extends Observable {}

IntervalWindow类需要与此崭新的领域类建立一个关联：

private Interval _subject;

然后，我需要合理地初始化_subject字段，并把IntervalWindow变成Interval的一个Observer。这很简单，只需把下列代码放进IntervalWindow构造函数中就可以了：

_subject = new Interval();

_subject.addObserver(this);

update(_subject,null);

我喜欢把这段代码放在整个构造过程的最后。其中对update()的调用可以确保：当我把数据复制到领域类后，GUI将根据领域类进行初始化。update()是在java.util.Observer接口中声明的，因此我必须让IntervalWindow实现这一接口：

public class IntervalWindow extends Frame implements Observer

然后我还需要为IntervalWindow类建立一个update()。此刻我先给它一个空的实现：

public void update(Observable observed,Object arg){

}

现在我可以编译并测试了。到目前为止我还没有做出任何真正的修改。呵呵，小心驶得万年船。

接下来，我把注意力转移到文本框。一如往常，我每次只改动一个字段。为了卖弄一下我的英语能力
[2]

 ，我就从End文本框开始。第一件要做的事就是实施Self Encapsulate Field (171)。文本框的更新是通过getText()和setText()两函数实现的，因此我所建立的访问函数需要调用这两个函数：

String getEnd(){

return _endField.getText();

}

void setEnd (String arg){

_endField.setText(arg);

}

然后，找出_endField的所有引用点，将它们替换为适当的访问函数：

void calculateLength(){

try {

int start = Integer.parseInt(_startField.getText());

int end = Integer.parseInt(getEnd());

int length = end - start;

_lengthField.setText(String.valueOf(length));

} catch (NumberFormatException e){

throw new RuntimeException("Unexpected Number Format Error");

}

}

void calculateEnd(){

try {

int start = Integer.parseInt(_startField.getText());

int length = Integer.parseInt(_lengthField.getText());

int end = start + length;

setEnd(String.valueOf(end));

} catch (NumberFormatException e){

throw new RuntimeException("Unexpected Number Format Error");

}

}

void EndField_FocusLost(java.awt.event.FocusEvent event){

if (isNotInteger(getEnd()))

setEnd("0");

calculateLength();

}

这是Self Encapsulate Field (171)的标准过程。然而当你处理GUI 时，情况还更复杂些：用户可以直接（通过GUI）修改文本框内容，不必调用setEnd()。因此我需要在GUI 的事件处理函数中调用setEnd()。这个动作把End文本框设定为其当前值。当然，这没带来什么影响，但是通过这样的方式，可以确保用户的输入确实是通过设值函数进行的：

void EndField_FocusLost(java.awt.event.FocusEvent event){

setEnd(_endField.getText());

if (isNotInteger(getEnd()))

setEnd("0");

calculateLength();

}

上述调用动作中，我并没有使用前面的getEnd()取得End文本框当前内容，而是直接访问文本框。之所以这样做是因为，随后的重构将使getEnd()从领域对象（而非文本框）身上取值。那时如果这里用的是getEnd()函数，每当用户修改文本框内容，这里就会将文本框又改回原值。所以我必须使用直接访问文本框的方式获取当前值。现在我可以编译并测试字段封装后的行为了。

现在，在领域类中加入_end字段：

class Interval...

private String _end = "0";

在这里，我给它的初值和GUI给它的初值是一样的。然后我再加入取值/设值函数：

class Interval...

String getEnd(){

return _end;

}

void setEnd (String arg){

_end = arg;

setChanged();

notifyObservers();

}

由于使用了Observer模式，我必须在设值函数中发出通告。我把_end声明为一个字符串，而不是一个看似更合理的整数，这是因为我希望将修改量减至最少。将来成功复制数据完毕后，我可以轻松地在领域类内部把_end声明为整数。

现在，我可以再编译并测试一次。我希望通过所有这些预备工作，将下面这个较为棘手的重构步骤的风险降至最低。

首先，修改IntervalWindow类的访问函数，令它们改用Interval对象：

class IntervalWindow...

String getEnd(){

return _subject.getEnd();

}

void setEnd (String arg){

_subject.setEnd(arg);

}

同时也修改update()函数，确保GUI对Interval对象发来的通告做出响应：

class IntervalWindow...

public void update(Observable observed,Object arg){

_endField.setText(_subject.getEnd());

}

这是另一个需要直接访问文本框的地点。如果我调用的是设值函数，程序将陷入无限递归调用。

现在，我可以编译并测试。数据都恰如其分地被复制了。

另两个文本框也如法炮制。完成之后，我可以使用Move Method (142)将calculateEnd()和calculateLength()搬到Interval去。这么一来，我就拥有一个包容所有领域行为和领域数据、并与GUI 分离的领域类了。

如果上述工作都完成了，我就会考虑彻底摆脱这个GUI类。如果它是个较为老旧的AWT类，我会考虑将它换成一个比较好看的Swing类 ，而且后者的坐标定位能力也比较强。我可以在领域类之上建立一个Swing GUI。这样，只要我高兴，随时可以去掉老旧的GUI 。

使用事件监听器

如果你使用事件监听器而不是Observer/Observable模式，仍然可以实施Duplicate Observed Data (189)。这种情况下，你需要在领域模型中建立一个监听器类和一个事件类（如果你不在意依赖关系的话，也可以使用AWT类）。然后，你需要对领域对象注册监听器，就像前例对observable对象注册observer一样。每当领域对象发生变化（类似上例的update()函数被调用），就向监听器发送一个事件。IntervalWindow可以利用一个内嵌类来实现监听器接口，并在适当时候调用适当的update()函数。

---

## 8.7 Change Unidirectional Association to Bidirectional （将单向关联改为双向关联）

两个类都需要使用对方特性，但其间只有一条单向连接。

添加一个反向指针，并使修改函数
[3]

 能够同时更新两条连接。

动机

开发初期，你可能会在两个类之间建立一条单向连接，使其中一个类可以引用另一个类。随着时间推移，你可能发现被引用类需要得到其引用者以便进行某些处理。也就是说，它需要一个反向指针。但指针是一种单向连接，你不可能反向操作它。通常你可以绕道而行，虽然会耗费一些计算时间，成本还算合理，然后你可以在被引用类中建立一个函数专门负责此一行为。但是，有时候想绕过这个问题并不容易，此时就需要建立双向引用关系，或称为反向指针。如果使用不当，反向指针很容易造成混乱；但只要你习惯了这种手法，它们其实并不是太复杂。

“反向指针”手法有点棘手，所以在你能够自如运用之前，应该有相应的测试。通常我不花心思去测试访问函数，因为普通访问函数的风险没有高到需要测试的地步，但本重构要求测试访问函数，所以它是极少数需要添加测试的重构手法之一。

本重构运用反向指针实现双向关联。其他技术（例如连接对象）需要其他重构手法。

做法

在被引用类中增加一个字段，用以保存反向指针。

决定由哪个类——引用端还是被引用端——控制关联关系。

在被控端建立一个辅助函数，其命名应该清楚指出它的有限用途。

如果既有的修改函数在控制端，让它负责更新反向指针。

如果既有的修改函数在被控端，就在控制端建立一个控制函数，并让既有的修改函数调用这个新建的控制函数。

范例

下面是一段简单程序，其中有两个类：表示“定单”的Order和表示“客户”的Customer。Order引用了Customer，Customer并没有引用Order：

class Order...

Customer getCustomer(){

return _customer;

}

void setCustomer (Customer arg){

_customer = arg;

}

Customer _customer;

首先，我要为Customer添加一个字段。由于一个客户可以拥有多份定单，所以这个新增字段应该是个集合。我不希望同一份定单在同一个集合中出现一次以上，所以这里适合使用set:

class Customer {

private Set _orders = new HashSet();

现在，我需要决定由哪一个类负责控制关联关系 。我比较喜欢让单个类来操控，因为这样就可以将所有处理关联关系的逻辑集中安置于一地。我将按照下列步骤做出这一决定。

1.如果两者都是引用对象，而其间的关联是“一对多”关系，那么就由“拥有单一引用”的那一方承担“控制者”角色。以本例而言，如果一个客户可拥有多份定单，那么就由Order类（定单）来控制关联关系。

2.如果某个对象是组成另一对象的部件，那么由后者负责控制关联关系。

3.如果两者都是引用对象s，而其间的关联是“多对多”关系，那么随便其中哪个对象来控制关联关系，都无所谓。

本例之中，由于Order负责控制关联关系，所以我必须为Customer添加一个辅助函数，让Order可以直接访问_orders（订单）集合。Order的修改函数将使用这个辅助函数对指针两端对象进行同步控制。我将这个辅助函数命名为friendOrders()，表示这个函数只能在这种特殊情况下使用。此外，如果Order和Customer位在同一个包内，我还会将friendOrders()声明为包内可见
[4]

 ，使其可见程度降到最低。但如果这两个类不在同一个包内，我就只好把friendOrders()声明为public了。

class Customer...

Set friendOrders(){

/** should only be used by Order when modifying the association */

return _orders;

}

现在，我要改变修改函数，令它同时更新反向指针：

class Order...

void setCustomer(Customer arg){

if (_customer != null)

_customer.friendOrders().remove(this);

_customer = arg;

if (_customer != null)

_customer.friendOrders().add(this);

}

类之间的关联关系是各式各样的，因此修改函数的代码也会随之有所差异。如果_customer的值不可能是null，那么可以拿掉上述的第一个null检查，但仍然需要检查传入参数是否为null。不过，基本形式总是相同的：先让对方删除指向你的指针，再将你的指针指向一个新对象，最后让那个新对象把它的指针指向你。

如果你希望在Customer中也能修改连接，就让它调用控制函数：

class Customer...

void addOrder(Order arg){

arg.setCustomer(this);

}

如果一份定单也可以对应多个客户，那么你所面临的就是一个“多对多”情况，重构后的函数可能是下面这样：

class Order...//controlling methods

void addCustomer(Customer arg){

arg.friendOrders().add(this);

_customers.add(arg);

}

void removeCustomer(Customer arg){

arg.friendOrders().remove(this);

_customers.remove(arg);

}

class Customer...

void addOrder(Order arg){

arg.addCustomer(this);

}

void removeOrder(Order arg){

arg.removeCustomer(this);

}

---

## 8.8 Change Bidirectional Association to Unidirectional （将双向关联改为单向关联）

两个类之间有双向关联，但其中一个类如今不再需要另一个类的特性。

去除不必要的关联。

动机

双向关联很有用，但你也必须为它付出代价，那就是维护双向连接、确保对象被正确创建和删除而增加的复杂度。而且，由于很多程序员并不习惯使用双向关联，它往往成为错误之源。

大量的双向连接也很容易造成“僵尸对象”：某个对象本来已经该死亡了，却仍然保留在系统中，因为对它的引用还没有完全清除。

此外，双向关联也迫使两个类之间有了依赖：对其中任一个类的任何修改，都可能引发另一个类的变化。如果这两个类位于不同的包，这种依赖就是包与包之间的相依。过多的跨包依赖会造就紧耦合系统，使得任何一点小小改动都可能造成许多无法预知的后果。

只有在真正需要双向关联的时候，才应该使用它。如果发现双向关联不再有存在价值，就应该去掉其中不必要的一条关联。

做法

找出保存“你想去除的指针”的字段，检查它的每一个用户，判断是否可以去除该指针。

不但要检查直接访问点，也要检查调用这些直接访问点的函数。

考虑有无可能不通过指针取得被引用对象。如果有可能，你就可以对取值函数使用Substitute Algorithm (139)，从而让客户在没有指针的情况下也可以使用该取值函数。

对于使用该字段的所有函数，考虑将被引用对象作为参数传进去。

如果客户使用了取值函数，先运用Self Encapsulate Field (171)将待删除字段自我封装起来，然后使用Substitute Algorithm (139)对付取值函数，令它不再使用该字段。然后编译、测试。

如果客户并未使用取值函数，那就直接修改待删除字段的所有被引用点：改以其他途径获得该字段所保存的对象。每次修改后，编译并测试。

如果已经没有任何函数使用待删除字段，移除所有对该字段的更新逻辑，然后移除该字段。

如果有许多地方对此字段赋值，先运用Self Encapsulate Field (171)使这些地点改用同一个设值函数。编译、测试。而后将这个设值函数的本体清空。再编译、再测试。如果这些都可行，就可以将此字段和其设值函数，连同对设值函数的所有调用，全部移除。

编译，测试。

范例

本例从Change Unidirectional Association to Bidirectional (197)留下的代码开始进行，其中Customer和Order之间有双向关联：

class Order...

Customer getCustomer(){

return _customer;

}

void setCustomer(Customer arg){

if (_customer != null)

_customer.friendOrders().remove(this);

_customer = arg;

if (_customer != null)

_customer.friendOrders().add(this);

}

private Customer _customer;

class Customer...

void addOrder(Order arg){

arg.setCustomer(this);

}

private Set _orders = new HashSet();

Set friendOrders(){

/** should only be used by Order */

return _orders;

}

后来我发现，除非先有Customer对象，否则不会存在Order对象。因此我想将从Order到Customer的连接移除掉。

对于本项重构来说，最困难的就是检查可行性。如果我知道本项重构是安全的，那么重构手法自身十分简单。问题在于是否有任何代码依赖_customer字段存在。如果确实有，那么在删除这个字段之后，必须提供替代品。

首先，我需要研究所有读取这个字段的函数，以及所有使用这些函数的函数。我能找到另一条途径来提供Customer对象吗——这通常意味着将Customer对象作为参数传递给用户。下面是一个简化例子：

class Order...

double getDiscountedPrice(){

return getGrossPrice()* (1 - _customer.getDiscount());

}

改变为：

class Order...

double getDiscountedPrice(Customer customer){

return getGrossPrice()* (1 - customer.getDiscount());

}

如果待改函数是被Customer对象调用的，那么这样的修改方案特别容易实施，因为Customer对象将自己作为参数传给函数很容易。所以下列代码：

class Customer...

double getPriceFor(Order order){

Assert.isTrue(_orders.contains(order)); // see Introduce Assertion (267)

return order.getDiscountedPrice();

变成了：

class Customer...

double getPriceFor(Order order){

Assert.isTrue(_orders.contains(order));

return order.getDiscountedPrice(this);

}

另一种做法就是修改取值函数，使其在不使用_customer字段的前提下返回一个Customer对象。如果这行得通，就可以使用Substitute Algorithm (139)修改Order.getCustomer()函数算法。我有可能这样修改代码：

Customer getCustomer(){

Iterator iter = Customer.getInstances().iterator();

while (iter.hasNext()){

Customer each = (Customer)iter.next();

if (each.containsOrder(this))return each;

}

return null;

}

这段代码比较慢，不过确实可行。而且，在数据库环境下，如果我需要使用数据库查询语句，这段代码对系统性能的影响可能并不显著。如果Order 类中有些函数使用_customer字段，我可以实施Self Encapsulate Field (171)令它们转而改用上述的getCustomer()函数。

如果我要保留上述的取值函数，那么Order和Customer的关联从接口上看虽然仍是双向的，但实现上已经是单向关系了。虽然我移除了反向指针，但两个类彼此之间的依赖关系仍然存在。

既然要替换取值函数，那么我就专注地替换它，其他部分留待以后处理。我会逐一修改取值函数的调用者，让它们通过其他来源取得Customer对象。每次修改后都编译并测试。实际工作中这一过程往往相当快。如果这个过程让我觉得很棘手很复杂，我会放弃本项重构。

一旦消除了_customer字段的所有读取点，我就可以着手处理对此字段赋值的函数了。很简单，只要把这些赋值动作全部移除，再把字段一并删除就行了。由于已经没有任何代码需要这个字段，所以删掉它并不会带来任何影响。

---

## 8.9 Replace Magic Number with Symbolic Constant （以字面常量取代魔法数）

你有一个字面数值，带有特别含义。

创造一个常量，根据其意义为它命名，并将上述的字面数值替换为这个常量。

double potentialEnergy(double mass,double height){
return mass * 9.81 * height;
}

double potentialEnergy(double mass,double height){
return mass * GRAVITATIONAL_CONSTANT * height;
}
static final double GRAVITATIONAL_CONSTANT = 9.81;

动机

在计算科学中，魔法数（magic number）是历史最悠久的不良现象之一。所谓魔法数是指拥有特殊意义，却又不能明确表现出这种意义的数字。如果你需要在不同的地点引用同一个逻辑数，魔法数会让你烦恼不己，因为一旦这些数发生改变，你就必须在程序中找到所有魔法数，并将它们全部修改一遍，这简直就是一场噩梦。就算你不需要修改，要准确指出每个魔法数的用途，也会让你颇费脑筋。

许多语言都允许你声明常量。常量不会造成任何性能开销，却可以大大提高代码的可读性。

进行本项重构之前，你应该先寻找其他替换方案。你应该观察魔法数如何被使用，而后你往往会发现一种更好的使用方式。如果这个魔法数是个类型码，请考虑使用Replace Type Code with Class (218)；如果这个魔法数代表一个数组的长度，请在遍历该数组的时候，改用Array.length()。

做法

声明一个常量，令其值为原本的魔法数值。

找出这个魔法数的所有引用点。

检查是否可以使用这个新声明的常量来替换该魔法数。如果可以，便以此常量替换之。

编译。

所有魔法数都被替换完毕后，编译并测试。此时整个程序应该运转如常，就像没有做任何修改一样。

有个不错的测试办法：检查现在的程序是否可以被你轻松地修改常量值（这可能意味某些预期结果将有所改变，以配合这一新值。实际工作中并非总是可以进行这样的测试）。如果可行，这就是一个不错的手法。

---

## 8.10 Encapsulate Field（封装字段）

你的类中存在一个public字段。

将它声明为private，并提供相应的访问函数。

public String _name;

private String _name;
public String getName(){return _name;}
public void setName(String arg){_name = arg;}

动机

面向对象的首要原则之一就是封装，或者称为“数据隐藏” 。按此原则，你绝不应该将数据声明为public，否则其他对象就有可能访问甚至修改这项数据，而拥有该数据的对象却毫无察觉。于是，数据和行为就被分开了——这可不是件好事。

数据声明为public被看做是一种不好的做法，因为这样会降低程序的模块化程度。数据和使用该数据的行为如果集中在一起，一旦情况发生变化，代码的修改就会比较简单，因为需要修改的代码都集中于同一块地方，而不是星罗棋布地散落在整个程序中。

Encapsulate Field (206)是封装过程的第一步。通过这项重构手法，你可以将数据隐藏起来，并提供相应的访问函数。但它毕竟只是第一步。如果一个类除了访问函数外不能提供其他行为，它终究只是一个哑吧类。这样的类并不能享受对象技术带来的好处。而你知道，浪费任何一个对象都是很不好的。实施Encapsulate Field (206)之后，我会尝试寻找用到新建访问函数的代码，看看是否可以通过简单的Move Method (142)轻快地将它们移到新对象去。

做法

为public字段提供取值/设值函数。

找到这个类以外使用该字段的所有地点。如果客户只是读取该字段，就把引用替换为对取值函数的调用；如果客户修改了该字段值，就将此引用点替换为对设值函数的调用。

如果这个字段是个对象，而客户只不过是调用该对象的某个函数，那么无论该函数是否改变对象状态，都只能算是读取该字段。只有当客户为该字段赋值时，才能将其替换为设值函数。

每次修改之后，编译并测试。

将字段的所有用户修改完毕后，把字段声明为private。

编译，测试。

---

## 8.11 Encapsulate Collection（封装集合）

有个函数返回一个集合。

让这个函数返回该集合的一个只读副本，并在这个类中提供添加/移除集合元素的函数。

动机

我们常常会在一个类中使用集合（collection，可能是array、list、set或vector）来保存一组实例。这样的类通常也会提供针对该集合的取值/设值函数。

但是，集合的处理方式应该和其他种类的数据略有不同。取值函数不该返回集合自身，因为这会让用户得以修改集合内容而集合拥有者却一无所悉。这也会对用户暴露过多对象内部数据结构的信息。如果一个取值函数确实需要返回多个值，它应该避免用户直接操作对象内所保存的集合，并隐藏对象内与用户无关的数据结构。至于如何做到这一点，视你使用的Java版本不同而有所不同。

另外，不应该为这整个集合提供一个设值函数，但应该提供用以为集合添加/移除元素的函数。这样，集合拥有者（对象）就可以控制集合元素的添加和移除。

如果你做到以上几点，集合就被很好地封装起来了，这便可以降低集合拥有者和用户之间的耦合度。

做法

加入为集合添加/移除元素的函数。

将保存集合的字段初始化为一个空集合。

编译。

找出集合设值函数的所有调用者。你可以修改那个设值函数，让它使用上述新建立的“添加/移除元素”函数；也可以直接修改调用端，改让它们调用上述新建立的“添加/移除元素”函数。

两种情况下需要用到集合设值函数：(1)集合为空时；(2)准备将原有集合替换为另一个集合时。

你或许会想运用Rename Method (273)为集合设值函数改名：从setXxx()改为initializeXxx()或replaceXxx()。

编译，测试。

找出所有“通过取值函数获得集合并修改其内容”的函数。逐一修改这些函数，让它们改用添加/移除函数。每次修改后，编译并测试。

修改完上述所有“通过取值函数获得集合并修改集合内容”的函数后，修改取值函数自身，使它返回该集合的一个只读副本。

在Java 2中，你可以使用Collection.unmodifiableXxx()得到该集合的只读副本。

在Java l.1中，你应该返回集合的一份副本。

编译，测试。

找出取值函数的所有用户，从中找出应该存在于集合所属对象内的代码。运用Extract Method (110)和Move Method (142)将这些代码移到宿主对象去。

如果你使用Java 2，那么本项重构到此为止。如果你使用Java l.1，那么用户也许会喜欢使用枚举。为了提供这个枚举，你应该像如下这样做。

修改现有取值函数的名字，然后添加一个新取值函数，使其返回一个枚举。找出旧取值函数的所有被使用点，将它们都改为使用新取值函数。

如果这一步跨度太大，你可以先使用Rename Method (273)修改原取值函数的名称；再建立一个新取值函数用以返回枚举；最后再修改所有调用者，使其调用新取值函数。

编译，测试。

范例

Java 2提供了全新的集合类——并非仅仅加入一些新类，而是完全改变了集合的风格。所以在Java l.1和Java 2中，封装集合的方式也完全不同。我首先讨论Java 2的方式，因为我认为功能更强大的Java 2 会取代Java l.1 的地位。

范例：Java 2

假设有个人要去上课。我们用一个简单的Course来表示“课程”：

class Course...

public Course (String name,boolean isAdvanced){...};

public boolean isAdvanced(){...};

我不关心课程其他细节。我感兴趣的是表示“人”的Person：

class Person...

public Set getCourses(){

return _courses;

}

public void setCourses(Set arg){

_courses = arg;

}

private Set _courses;

有了这个接口，我们就可以这样为某人添加课程：

Person kent = new Person();

Set s = new HashSet();

s.add(new Course ("Smalltalk Programming",false));

s.add(new Course ("Appreciating Single Malts",true));

kent.setCourses(s);

Assert.equals (2,kent.getCourses().size());

Course refact = new Course ("Refactoring",true);

kent.getCourses().add(refact);

kent.getCourses().add(new Course ("Brutal Sarcasm",false));

Assert.equals (4,kent.getCourses().size());

kent.getCourses().remove(refact);

Assert.equals (3,kent.getCourses().size());

如果想了解高级课程，可以这么做：

Iterator iter = person.getCourses().iterator();

int count = 0;

while (iter.hasNext()){

Course each = (Course)iter.next();

if (each.isAdvanced())count ++;

}

我要做的第一件事就是为Person中的集合建立合适的修改函数（即添加/移除函数），如下所示，然后编译：

class Person

public void addCourse (Course arg){

_courses.add(arg);

}

public void removeCourse (Course arg){

_courses.remove(arg);

}

如果像下面这样初始化_courses字段，我的人生会轻松得多：

private Set _courses = new HashSet();

接下来，我需要观察设值函数的调用者。如果有许多地点大量运用了设值函数，就需要修改设值函数，令它调用添加/移除函数。这个过程的复杂度取决于设值函数的被使用方式。设值函数的用法有两种，最简单的情况就是：它被用来初始化集合。换句话说，设值函数被调用之前，_courses是个空集合。这种情况下只需修改设值函数，令它调用添加函数就行了：

class Person...

public void setCourses(Set arg){

Assert.isTrue(_courses.isEmpty());

Iterator iter = arg.iterator();

while (iter.hasNext()){

addCourse((Course)iter.next());

}

}

修改完毕后，最好以Rename Method (273)更明确地展示这个函数的意图。

public void initializeCourses(Set arg){

Assert.isTrue(_courses.isEmpty());

Iterator iter = arg.iterator();

while (iter.hasNext()){

addCourse((Course)iter.next());

}

}

更普通的情况下
[5]

 ，我必须首先调用移除函数将集合中的所有元素全部移除，然后再调用添加函数将元素一一添加进去。不过我发现这种情况很少出现（唔，愈是普通的情况，愈少出现）。

如果我知道初始化时，除了添加元素，不会再有其他行为，那么我可以不使用循环，直接调用addAll()函数：

public void initializeCourses(Set arg){

Assert.isTrue(_courses.isEmpty());

_courses.addAll(arg);

}

我不能直接把传入的set赋值给_courses字段，就算原本这个字段是空的也不行。因为万一用户在把set传递给Person对象之后又去修改set中的元素，就会破坏封装。我必须像上面那样创建set的一个副本。

如果用户仅仅只是创建一个set，然后使用设值函数
[6]

 ，我可以让它们直接使用添加/移除函数，并将设值函数完全移除。于是，以下代码：

Person kent = new Person();

Set s = new HashSet();

s.add(new Course ("Smalltalk Programming",false));

s.add(new Course ("Appreciating Single Malts",true));

kent.initializeCourses(s);

就变成了：

Person kent = new Person();

kent.addCourse(new Course ("Smalltalk Programming",false));

kent.addCourse(new Course ("Appreciating Single Malts",true));

接下来，我开始观察取值函数的使用情况。首先处理“通过取值函数修改集合元素”的情况，例如：

kent.getCourses().add(new Course ("Brutal Sarcasm",false));

这种情况下我必须加以改变，使它调用新的修改函数：

kent.addCourse(new Course ("Brutal Sarcasm",false));

修改完所有此类情况之后，我可以让取值函数返回一个只读副本，用以确保没有任何一个用户能够通过取值函数修改集合：

public Set getCourses(){

return Collections.unmodifiableSet(_courses);

}

这样我就完成了对集合的封装。此后，不通过Person提供的添加/移除函数，谁也不能修改集合内的元素。

将行为移到这个类中

我拥有了合理的接口。现在开始观察取值函数的用户，从中找出应该属于Person的代码。下面这样的代码就应该搬移到Person去：

Iterator iter = person.getCourses().iterator();

int count = 0;

while (iter.hasNext()){

Course each = (Course)iter.next();

if (each.isAdvanced())count ++;

}

因为以上只使用了属于Person的数据。首先我使用Extract Method (110)将这段代码提炼为一个独立函数：

int numberOfAdvancedCourses(Person person){

Iterator iter = person.getCourses().iterator();

int count = 0;

while (iter.hasNext()){

Course each = (Course)iter.next();

if (each.isAdvanced())count++;

}

return count;

}

然后使用Move Method (142)将这个函数搬移到Person中：

class Person...

int numberOfAdvancedCourses(){

Iterator iter = getCourses().iterator();

int count = 0;

while (iter.hasNext()){

Course each = (Course)iter.next();

if (each.isAdvanced())count++;

}

return count;

}

下列代码是一个常见的例子 ：

kent.getCourses().size()

可以将其修改成更具可读性的样子，像这样：

kent.numberOfCourses()

class Person...

public int numberOfCourses(){

return _courses.size();

}

数年以前，我曾经担心将这样的行为搬移到Person中会导致Person变得臃肿。但是在实际工作经验中，我发现这通常并不成为问题。

范例：Java 1.1

在很多地方，Java l.1的情况和Java 2非常相似。这里我使用同一个范例，不过集合改为vector
[7]

 ：

class Person...

public Vector getCourses(){

return _courses;

}

public void setCourses(Vector arg){

_courses = arg;

}

private Vector _courses;

同样地，我首先建立修改函数，并初始化_courses字段，如下所示：

class Person

public void addCourse(Course arg){

_courses.addElement(arg);

}

public void removeCourse(Course arg){

_courses.removeElement(arg);

}

private Vector _courses = new Vector();

我可以修改setCourses()来初始化这个vector：

public void initializeCourses(Vector arg){

Assert.isTrue(_courses.isEmpty());

Enumeration e = arg.elements();

while (e.hasMoreElements()){

addCourse((Course)e.nextElement());

}

}

然后，我修改取值函数调用点，让它们改用新建的修改函数。于是下列代码：

kent.getCourses().addElement(new Course ("Brutal Sarcasm",false));

就变成了：

kent.addCourse(new Course ("Brutal Sarcasm",false));

最后一步需要有点改变，因为Java 1.1的Vector类并没有提供“不可修改版”:

class Person...

Vector getCourses(){

return (Vector)_courses.clone();

}

这样便完成了集合的封装。此后，如果不通过Person提供的函数，谁也不能改变集合的元素。

范例：封装数组

数组经常被使用，特别是对于那些不熟悉集合的程序员而言。我很少使用数组，因为我更喜欢功能更加丰富的集合类。进行封装时，我常把数组换成其他集合。

这次我们的范例从一个字符串数组开始：

String[] getSkills(){

return _skills;

}

void setSkills (String[] arg){

_skills = arg;

}

String[] _skills;

同样地，首先要提供一个修改函数。由于用户有可能修改数组中某一特定位置上的值，所以我提供的setSkill()必须能对任何特定位置上的元素赋值：

void setSkill(int index,String newSkill){

_skills[index] = newSkill;

}

如果我需要对整个数组赋值，可以使用下列函数：

void setSkills(String[] arg){

_skills = new String[arg.length];

for (int i = 0; i < arg.length; i++)

setSkill(i,arg[i]);

}

如果需要处理从数组中移除元素，就会有些困难。如果作为参数传入的数组和原数组长度不同，情况也会比较复杂。这也是我优先选择集合的原因之一。

现在，我需要观察取值函数的调用者。我可以把下列代码：

kent.getSkills()[1] = "Refactoring";

改成：

kent.setSkill(1,"Refactoring");

完成这一系列修改之后，我可以修改取值函数，令它返回一份数组副本：

String[] getSkills(){

String[] result = new String[_skills.length];

System.arraycopy(_skills,0,result,0,_skills.length);

return result;

}

现在，是把数组换成list的时候了：

class Person...

String[] getSkills(){

return (String[])_skills.toArray(new String[0]);

}

void setSkill(int index,String newSkill){

_skills.set(index,newSkill);

}

List _skills = new ArrayList();

---

## 8.12 Replace Record with Data Class（以数据类取代记录）

你需要面对传统编程环境中的记录结构。

为该记录创建一个“哑”数据对象。

动机

记录型结构是许多编程环境的共同性质。有一些理由使它们被带进面向对象程序之中：你可能面对的是一个遗留程序，也可能需要通过一个传统API来与记录结构交流，或是处理从数据库读出的记录。这些时候你就有必要创建一个接口类，用以处理这些外来数据。最简单的做法就是先建立一个看起来类似外部记录的类，以便日后将某些字段和函数搬移到这个类之中。一个不太常见但非常令人注目的情况是：数组中的每个位置上的元素都有特定含义，这种情况下应该使用Replace Array with Object (186)。

做法

新建一个类，表示这个记录。

对于记录中的每一项数据，在新建的类中建立对应的一个private字段，并提供相应的取值/设值函数。

现在，你拥有了一个“哑”数据对象。这个对象现在还没有任何有用的行为，但是更进一步的重构会解决这个问题。

---

## 8.13 Replace Type Code with Class（以类取代类型码）

类之中有一个数值类型码，但它并不影响类的行为。

以一个新的类替换该数值类型码。

动机

在以C为基础的编程语言中，类型码或枚举值很常见。如果带着一个有意义的符号名，类型码的可读性还是不错的。问题在于，符号名终究只是个别名，编译器看见的、进行类型检验的，还是背后那个数值。任何接受类型码作为参数的函数，所期望的实际上是一个数值，无法强制使用符号名。这会大大降低代码的可读性，从而成为bug之源。

如果把那样的数值换成一个类，编译器就可以对这个类进行类型检验。只要为这个类提供工厂函数，你就可以始终保证只有合法的实例才会被创建出来，而且它们都会被传递给正确的宿主对象。

但是，在使用Replace Type Code with Class (218)之前，你应该先考虑类型码的其他替换方式。只有当类型码是纯粹数据时（也就是类型码不会在switch语句中引起行为变化时），你才能以类来取代它。Java只能以整数作为switch语句的判断依据，不能使用任意类，因此那种情况下不能够以类替换类型码。更重要的是：任何switch语句都应该运用Replace Conditional with Polymorphism (255)去掉。为了进行那样的重构，你首先必须运用Replace Type Code with Subclasses (223)或Replace Type Code with State/Strategy (227)，把类型码处理掉。

即使一个类型码不会因其数值的不同而引起行为上的差异，宿主类中的某些行为还是有可能更适合置放于类型码类中，因此你还应该留意是否有必要使用Move Method (142)将一两个函数搬过去。

做法

为类型码建立一个类。

这个类需要一个用以记录类型码的字段，其类型应该和类型码相同，并应该有对应的取值函数。此外还应该用一组静态变量保存允许被创建的实例，并以一个静态函数根据原本的类型码返回合适的实例。

修改源类实现，让它使用上述新建的类。

维持原先以类型码为基础的函数接口，但改变静态字段，以新建的类产生代码。然后，修改类型码相关函数，让它们也从新建的类中获取类型码。

编译，测试。

此时，新建的类可以对类型码进行运行期检查。

对于源类中每一个使用类型码的函数，相应建立一个函数，让新函数使用新建的类。

你需要建立“以新类实例为自变量”的函数，用以替换原先“直接以类型码为参数”的函数。你还需要建立一个“返回新类实例”的函数，用以替换原先“直接返回类型码”的函数。建立新函数前，你可以使用Rename Method (273)修改原函数名称，明确指出哪些函数仍然使用旧式的类型码，这往往是个明智之举。

逐一修改源类用户，让它们使用新接口。

每修改一个用户，编译并测试。

你也可能需要一次性修改多个彼此相关的函数，才能保持这些函数之间的一致性，才能顺利地编译、测试。

删除使用类型码的旧接口，并删除保存旧类型码的静态变量。

编译，测试。

范例

每个人都拥有四种血型中的一种。我们以Person来表示“人”，以其中的类型码表示“血型”：

class Person {

public static final int O = 0;

public static final int A = 1;

public static final int B = 2;

public static final int AB = 3;

private int _bloodGroup;

public Person(int bloodGroup){

_bloodGroup = bloodGroup;

}

public void setBloodGroup(int arg){

_bloodGroup = arg;

}

public int getBloodGroup(){

return _bloodGroup;

}

}

首先，我建立一个新的BloodGroup类，用以表示“血型”，并在这个类实例中保存原本的类型码数值：

class BloodGroup {

public static final BloodGroup O = new BloodGroup(0);

public static final BloodGroup A = new BloodGroup(1);

public static final BloodGroup B = new BloodGroup(2);

public static final BloodGroup AB = new BloodGroup(3);

private static final BloodGroup[] _values = { O,A,B,AB };

private final int _code;

private BloodGroup(int code){

_code = code;

}

public int getCode(){

return _code;

}

public static BloodGroup code(int arg){

return _values[arg];

}

}

然后，我把Person中的类型码改为使用BloodGroup类：

class Person {

public static final int O = BloodGroup.O.getCode();

public static final int A = BloodGroup.A.getCode();

public static final int B = BloodGroup.B.getCode();

public static final int AB = BloodGroup.AB.getCode();

private BloodGroup _bloodGroup;

public Person(int bloodGroup){

_bloodGroup = BloodGroup.code(bloodGroup);

}

public int getBloodGroup(){

return _bloodGroup.getCode();

}

public void setBloodGroup(int arg){

_bloodGroup = BloodGroup.code(arg);

}

}

现在，我因为BloodGroup类而拥有了运行期检验能力。为了真正从这些改变中获利，我还必须修改Person的用户，让它们以BloodGroup对象表示类型码，而不再使用整数。

首先，我使用Rename Method (273)修改类型码访问函数的名称，说明当前情况：

class Person...

public int getBloodGroupCode(){

return _bloodGroup.getCode();

}

然后我为Person加入一个新的取值函数，其中使用BloodGroup：

public BloodGroup getBloodGroup(){

return _bloodGroup;

}

另外，我还要建立新的构造函数和设值函数，让它们也使用BloodGroup：

public Person (BloodGroup bloodGroup ){

_bloodGroup = bloodGroup;

}

public void setBloodGroup(BloodGroup arg){

_bloodGroup = arg;

}

现在，我要继续处理Person用户。此时应该注意，每次只处理一个用户，这样才可以保持小步前进。每个用户需要的修改方式可能不同，这使得修改过程更加棘手。对Person内的静态变量的所有引用点也需要修改。因此，下列代码：

Person thePerson = new Person(Person.A)

就变成了：

Person thePerson = new Person(BloodGroup.A);

原来调用取值函数的代码必须改为调用BloodGroup的取值函数。因此，下列代码：

thePerson.getBloodGroupCode()

变成了：

thePerson.getBloodGroup().getCode()

设值函数也一样。因此，下列代码：

thePerson.setBloodGroup(Person.AB)

变成了：

thePerson.setBloodGroup(BloodGroup.AB)

修改完毕Person的所有用户之后，我就可以删掉原本使用整数类型的那些旧的取值函数、构造函数、静态变量和设值函数了：

class Person ...

public static final int O = BloodGroup.O.getCode();

public static final int A = BloodGroup.A.getCode();

public static final int B = BloodGroup.B.getCode();

public static final int AB = BloodGroup.AB.getCode();

public Person (int bloodGroup){

_bloodGroup = BloodGroup.code(bloodGroup);

}

public int getBloodGroupCode(){

return _bloodGroup.getCode();

}

public void setBloodGroup(int arg){

_bloodGroup = BloodGroup.code (arg);

}

我还可以将BloodGroup中使用整数类型的函数声明为private，因为再没有人会使用它们了：

class BloodGroup...

private int getCode(){

return _code;

}

private static BloodGroup code(int arg){

return _values[arg];

}

---

## 8.14 Replace Type Code with Subclasses（以子类取代类型码）

你有一个不可变的类型码，它会影响类的行为。

以子类取代这个类型码。

动机

如果你面对的类型码不会影响宿主类的行为，可以使用Replace Type Code with Class (218)来处理它们。但如果类型码会影响宿主类的行为，那么最好的办法就是借助多态来处理变化行为。

一般来说，这种情况的标志就是像switch这样的条件表达式。这种条件表达式可能有两种表现形式：switch语句或者if-then-else结构。不论哪种形式，它们都是检查类型码值，并根据不同的值执行不同的动作。这种情况下，你应该以Replace Conditional with Polymorphism (255)进行重构。但为了能够顺利进行那样的重构，首先应该将类型码替换为可拥有多态行为的继承体系。这样的一个继承体系应该以类型码的宿主类为基类，并针对每一种类型码各建立一个子类。

为建立这样的继承体系，最简单的办法就是Replace Type Code with Subclasses (223)：以类型码的宿主类为基类，针对每种类型码建立相应的子类。

但是以下两种情况你不能那么做：(1)类型码值在对象创建之后发生了改变；(2)由于某些原因，类型码宿主类已经有了子类。如果你恰好面临这两种情况之一，就需要使用Replace Type Code with State/Strategy (227)。

Replace Type Code with Subclasses (223)的主要作用其实是搭建一个舞台，让Replace Conditional with Polymorphism (255)得以一展身手。如果宿主类中并没有出现条件表达式，那么Replace Type Code with Class (218)更合适，风险也比较低。

使用Replace Type Code with Subclasses (223)的另一个原因就是，宿主类中出现了“只与具备特定类型码之对象相关”的特性。完成本项重构之后，你可以使用Push Down Method (328)和Push Down Field (329)将这些特性推到合适的子类去，以彰显它们只与特定情况相关这一事实。

Replace Type Code with Subclasses (223)的好处在于：它把“对不同行为的了解”从类用户那儿转移到了类自身。如果需要再加入新的行为变化，只需添加一个子类就行了。如果没有多态机制，就必须找到所有条件表达式，并逐一修改它们。因此，如果未来还有可能加入新行为，这项重构将特别有价值。

做法

使用Self Encapsulate Field (171)将类型码自我封装起来。

如果类型码被传递给构造函数，就需要将构造函数换成工厂函数。

为类型码的每一个数值建立一个相应的子类。在每个子类中覆写类型码的取值函数，使其返回相应的类型码值。

这个值被硬编码于return句中（例如，return 1）。这看起来很肮脏，但只是权宜之计。当所有case子句都被替换后，问题就解决了。

每建立一个新的子类，编译并测试。

从超类中删掉保存类型码的字段。将类型码访问函数声明为抽象函数。

编译，测试。

范例

为简单起见，我还是使用那个恼人又不切实际的“雇员/薪资”例子。我们以Employee表示“雇员”：

class Employee...

private int _type;

static final int ENGINEER = 0;

static final int SALESMAN = 1;

static final int MANAGER = 2;

Employee (int type){

_type = type;

}

第一步是以Self Encapsulate Field (171)将类型码自我封装起来：

int getType(){

return _type;

}

由于Employee构造函数接受类型码作为一个参数，所以我必须将它替换为一个工厂函数:

static Employee create(int type){

return new Employee(type);

}

private Employee (int type){

_type = type;

}

现在，我可以先建立一个子类Engineer表示“工程师”。首先我建立这个子类，并在其中覆写类型码取值函数：

class Engineer extends Employee {

int getType(){

return Employee.ENGINEER;

}

}

同时我应该修改工厂函数，令它返回一个合适的对象：

class Employee

static Employee create(int type){

if (type == ENGINEER)return new Engineer();

else return new Employee(type);

}

然后，我继续逐一地处理其他类型码，直到所有类型码都被替换成子类为止。此时我就可以移除Employee中保存类型码的字段，并将getType()声明为一个抽象函数。现在，工厂函数看起来像这样：

abstract int getType();

static Employee create(int type){

switch (type){

case ENGINEER:

return new Engineer();

case SALESMAN:

return new Salesman();

case MANAGER:

return new Manager();

default:

throw new IllegalArgumentException("Incorrect type code value");

}

}

当然，我总是避免使用switch语句。但这里只有一处用到switch语句，并且只用于决定创建何种对象，这样的switch语句是可以接受的。

很自然地，在建立了这些子类之后，你就应该使用Push Down Method (328)和Push Down Field (329)，将只与特定种类雇员相关的函数和字段推到相关的子类去。

---

## 8.15 Replace Type Code with State/Strategy （以State/Strategy取代类型码）

你有一个类型码，它会影响类的行为，但你无法通过继承手法消除它。

以状态对象取代类型码。

动机

本项重构和Replace Type Code with Subclasses (223)很相似，但如果“类型码的值在对象生命期中发生变化”或“其他原因使得宿主类不能被继承”，你也可以使用本重构。本重构使用State模式或Strategy模式[Gang of Four]。

State模式和Strategy模式非常相似，因此无论你选择其中哪一个，重构过程都是相同的。“选择哪一个模式”并非问题关键所在，你只需要选择更适合特定情境的模式就行了。如果你打算在完成本项重构之后再以Replace Conditional with Polymorphism (255)简化一个算法，那么选择Strategy模式比较合适；如果你打算搬移与状态相关的数据，而且你把新建对象视为一种变迁状态，就应该选择使用State模式。

做法

使用Self Encapsulate Field (171)将类型码自我封装起来。

新建一个类，根据类型码的用途为它命名。这就是一个状态对象。

为这个新类添加子类，每个子类对应一种类型码。

比起逐一添加，一次性加入所有必要的子类可能更简单些。

在超类中建立一个抽象的查询函数，用以返回类型码。在每个子类中覆写该函数，返回确切的类型码。

编译。

在源类中建立一个字段，用以保存新建的状态对象。

调整源类中负责查询类型码的函数，将查询动作转发给状态对象。

调整源类中为类型码设值的函数，将一个恰当的状态对象子类赋值给“保存状态对象”的那个字段。

编译，测试。

范例

和上一项重构一样，我仍然使用这个既无聊又弱智的“雇员/薪资”例子。同样地，我以Employee表示“雇员”：

class Employee {

private int _type;

static final int ENGINEER = 0;

static final int SALESMAN = 1;

static final int MANAGER = 2;

Employee(int type){

_type = type;

}

下面的代码展示使用这些类型码的条件表达式：

int payAmount(){

switch (_type){

case ENGINEER:

return _monthlySalary;

case SALESMAN:

return _monthlySalary + _commission;

case MANAGER:

return _monthlySalary + _bonus;

default:

throw new RuntimeException("Incorrect Employee");

}

}

假设这是一家激情四溢、积极进取的公司，他们可以将表现出色的工程师擢升为经理。因此，对象的类型码是可变的，所以我不能使用继承方式来处理类型码。和以前一样，我的第一步还是使用Self Encapsulate Field (171)将表示类型码的字段自我封装起来：

Employee(int type){

setType(type);

}

int getType(){

return _type;

}

void setType(int arg){

_type = arg;

}

int payAmount(){

switch (getType()){

case ENGINEER:

return _monthlySalary;

case SALESMAN:

return _monthlySalary + _commission;

case MANAGER:

return _monthlySalary + _bonus;

default:

throw new RuntimeException("Incorrect Employee");

}

}

现在，我需要声明一个状态类。我把它声明为一个抽象类，并提供一个抽象函数，用以返回类型码：

abstract class EmployeeType {

abstract int getTypeCode();

}

现在，我可以开始创造子类了：

class Engineer extends EmployeeType {

int getTypeCode(){

return Employee.ENGINEER;

}

}

class Manager extends EmployeeType {

int getTypeCode(){

return Employee.MANAGER;

}

}

class Salesman extends EmployeeType {

int getTypeCode(){

return Employee.SALESMAN;

}

}

现在进行一次编译。前面所做的修改实在太平淡了，即使对我来说也太简单。现在，我要修改类型码访问函数，实实在在地把这些子类和Employee类联系起来：

class Employee...

private EmployeeType _type;

int getType(){

return _type.getTypeCode();

}

void setType(int arg){

switch (arg){

case ENGINEER:

_type = new Engineer();

break;

case SALESMAN:

_type = new Salesman();

break;

case MANAGER:

_type = new Manager();

break;

default:

throw new IllegalArgumentException("Incorrect Employee Code");

}

}

这意味我将在这里拥有一个switch语句。完成重构之后，这将是代码中唯一的switch语句，并且只在对象类型发生改变时才会执行。我也可以运用Replace Constructor with Factory Method (304)针对不同的case子句建立相应的工厂函数。我还可以立刻再使用Replace Conditional with Polymorphism (255)，从而将其他的case子句完全消除。

最后，我喜欢将所有关于类型码和子类的知识都移到新类，并以此结束Replace Type Code with State/Strategy (227)。首先，我把类型码的定义复制到EmployeeType去，在其中建立一个工厂函数以生成适当的EmployeeType对象，并调整Employee中为类型码赋值的函数：

class Employee...

void setType(int arg){

_type = EmployeeType.newType(arg);

}

class EmployeeType...

static EmployeeType newType(int code){

switch (code){

case ENGINEER:

return new Engineer();

case SALESMAN:

return new Salesman();

case MANAGER:

return new Manager();

default:

throw new IllegalArgumentException("Incorrect Employee Code");

}

}

static final int ENGINEER = 0;

static final int SALESMAN = 1;

static final int MANAGER = 2;

然后，我删掉Employee中的类型码定义，代之以一个指向EmployeeType对象的引用：

class Employee...

int payAmount(){

switch (getType()){

case EmployeeType.ENGINEER:

return _monthlySalary;

case EmployeeType.SALESMAN:

return _monthlySalary + _commission;

case EmployeeType.MANAGER:

return _monthlySalary + _bonus;

default:

throw new RuntimeException("Incorrect Employee");

}

}

现在，万事俱备，可以运用Replace Conditional with Polymorphism (255)来处理payAmount()函数了。

---

## 8.16 Replace Subclass with Fields（以字段取代子类）

你的各个子类的唯一差别只在“返回常量数据”的函数身上。

修改这些函数，使它们返回超类中的某个（新增）字段，然后销毁子类。

动机

建立子类的目的，是为了增加新特性或变化其行为。有一种变化行为被称为“常量函数”（constant method）[Beck]，它们会返回一个硬编码的值。这东西有其用途：你可以让不同的子类中的同一个访问函数返回不同的值。你可以在超类中将访问函数声明为抽象函数，并在不同的子类中让它返回不同的值。

尽管常量函数有其用途，但若子类中只有常量函数，实在没有足够的存在价值。你可以在超类中设计一个与常量函数返回值相应的字段，从而完全去除这样的子类。如此一来就可以避免因继承而带来的额外复杂性。

做法

对所有子类使用Replace Constructor with Factory Method (304)。

如果有任何代码直接引用子类，令它改而引用超类。

针对每个常量函数，在超类中声明一个final字段。

为超类声明一个protected构造函数，用以初始化这些新增字段。

新建或修改子类构造函数，使它调用超类的新增构造函数。

编译，测试。

在超类中实现所有常量函数，令它们返回相应字段值，然后将该函数从子类中删掉。

每删除一个常量函数，编译并测试。

子类中所有的常量函数都被删除后，使用Inline Method (117)将子类构造函数内联到超类的工厂函数中。

编译，测试。

将子类删掉。

编译，测试。

重复“内联构造函数、删除子类”过程，直到所有子类都被删除。

范例

本例之中，我以Person表示“人”，并针对每种性别建立一个子类：以Male子类表示“男人”，以Female子类表示“女人”：

abstract class Person {

abstract boolean isMale();

abstract char getCode();

...

class Male extends Person {

boolean isMale(){

return true;

}

char getCode(){

return 'M';

}

}

class Female extends Person {

boolean isMale(){

return false;

}

char getCode(){

return 'F';

}

}

在这里，两个子类之间唯一的区别就是：它们以不同的方式实现了Person所声明的抽象函数getCode()，返回不同的硬编码常量（所以getCode()是个常量函数[Beck]）。我应该将这两个怠惰的子类去掉。

首先我需要使用Replace Constructor with Factory Method (304)。在这里，我需要为每个子类建立一个工厂函数：

class Person...

static Person createMale(){

return new Male();

}

static Person createFemale(){

return new Female();

}

然后我把对象创建过程从以下这样：

Person kent = new Male();

改为这样：

Person kent = Person.createMale();

将所有调用构造函数的地方都改为调用工厂函数之后，就不应该再有任何对子类的直接引用了。一次全文搜索就可以帮助我证实这一点。然后，我可以把这两个子类都声明为private，这样编译器就可以帮助我，保证至少包外不会有任何代码使用它们。

现在，针对每个常量函数，在超类中声明一个对应的字段：

class Person...

private final boolean _isMale;

private final char _code;

然后为超类加上一个protected构造函数：

class Person...

protected Person (boolean isMale,char code){

_isMale = isMale;

_code = code;

}

再为子类加上新构造函数，令它调用超类新增的构造函数：

class Male...

Male(){

super (true,'M');

}

class Female...

Female(){

super (false,'F');

}

完成这一步后，编译并测试。所有字段都被创建出来并被赋予初值，但到目前为止，我们还没有使用它们。现在我可以在超类中加入访问这些字段的函数，并删掉子类中的常量函数，从而让这些字段粉墨登场：

class Person...

boolean isMale(){

return _isMale;

}

class Male...

boolean isMale(){

return true;

}

我可以逐一对每个字段、每个子类进行这一步骤的修改。如果我相信自己的运气，也可以采取一次性全部修改的手段。

所有字段都处理完毕后，所有子类也都空空如也了，于是可以删除Person中那个抽象函数的abstract修饰符，并以Inline Method (117)将子类构造函数内联到超类的工厂函数中：

class Person

static Person createMale(){

return new Person(true,'M');

}

编译、测试后，我就可以删掉Male类，并对Female类重复上述过程。

[1].
 这个重构手法特别复杂建议搭配范例阅读。——译者注

[2].
 “I’ll start with the end fieldy.”作者意指在这句话中一下子使用了start和end这两个反义词。——译者注

[3].
 modifier，指改变双方关系的函数。──译者注

[4].
 即不加任何修饰符的默认访问级别。——译者注

[5].
 指非上述所言对空集合设初值。——译者注

[6].
 目前已改名为initializeCourses()。——译者注

[7].
 因为vector属于Java 1.1，不属于Java 2。——译者注

---


# 第9章 简化条件表达式 (Simplifying Conditional Expressions)

## 9.1 Decompose Conditional（分解条件表达式）

你有一个复杂的条件（if-then-else）语句。

从if、then、else三个段落中分别提炼出独立函数。

if (date.before (SUMMER_START)|| date.after(SUMMER_END))
charge = quantity * _winterRate + _winterServiceCharge;
else charge = quantity * _summerRate;

if (notSummer(date))
charge = winterCharge(quantity);
else charge = summerCharge (quantity);

动机

程序之中，复杂的条件逻辑是最常导致复杂度上升的地点之一。你必须编写代码来检查不同的条件分支、根据不同的分支做不同的事，然后，你很快就会得到一个相当长的函数。大型函数自身就会使代码的可读性下降，而条件逻辑则会使代码更难阅读。在带有复杂条件逻辑的函数中，代码（包括检查条件分支的代码和真正实现功能的代码）会告诉你发生的事，但常常让你弄不清楚为什么会发生这样的事，这就说明代码的可读性的确大大降低了。

和任何大块头代码一样，你可以将它分解为多个独立函数，根据每个小块代码的用途，为分解而得的新函数命名，并将原函数中对应的代码改为调用新建函数，从而更清楚地表达自己的意图。对于条件逻辑，将每个分支条件分解成新函数还可以给你带来更多好处：可以突出条件逻辑，更清楚地表明每个分支的作用，并且突出每个分支的原因。

做法

将if段落提炼出来，构成一个独立函数。

将then段落和else段落都提炼出来，各自构成一个独立函数。

如果发现嵌套的条件逻辑，我通常会先观察是否可以使用Replace Nested Conditional with Guard Clauses (250)。如果不行，才开始分解其中的每个条件。

范例

假设我要计算购买某样商品的总价（总价=数量×单价），而这个商品在冬季和夏季的单价是不同的：

if (date.before (SUMMER_START)|| date.after(SUMMER_END))

charge = quantity * _winterRate + _winterServiceCharge;

else charge = quantity * _summerRate;

我把每个分支的判断条件都提炼到一个独立函数中，如下所示：

if (notSummer(date))

charge = winterCharge(quantity);

else charge = summerCharge (quantity);

private boolean notSummer(Date date){

return date.before (SUMMER_START)|| date.after(SUMMER_END);

}

private double summerCharge(int quantity){

return quantity * _summerRate;

}

private double winterCharge(int quantity){

return quantity * _winterRate + _winterServiceCharge;

}

通过这段代码可以看出整个重构带来的清晰性。实际工作中，我会逐步进行每一次提炼，并在每次提炼之后编译并测试。

像这样的情况下，许多程序员都不会去提炼分支条件。因为这些分支条件往往非常短，看上去似乎没有提炼的必要。但是，尽管这些条件往往很短，在代码意图和代码自身之间往往存在不小的差距。哪怕在上面这样一个小小例子中， notSummer(date)这个语句也能够比原本的代码更好地表达自己的用途。对于原来的代码，我必须看着它，想一想，才能说出其作用。当然，在这个简单的例子中，这并不困难。不过，即使如此，提炼出来的函数可读性也更高一些——它看上去就像一段注释那样清楚而明白。

---

## 9.2 Consolidate Conditional Expression（合并条件表达式）

你有一系列条件测试，都得到相同结果。

将这些测试合并为一个条件表达式，并将这个条件表达式提炼成为一个独立函数。

double disabilityAmount(){

if (_seniority < 2)return 0;

if (_monthsDisabled > 12)return 0;

if (_isPartTime)return 0;

// compute the disability amount

double disabilityAmount(){
if (isNotEligibleForDisability())return 0;
// compute the disability amount

动机

有时你会发现这样一串条件检查：检查条件各不相同，最终行为却一致。如果发现这种情况，就应该使用“逻辑或”和“逻辑与”将它们合并为一个条件表达式。

之所以要合并条件代码，有两个重要原因。首先，合并后的条件代码会告诉你“实际上只有一次条件检查，只不过有多个并列条件需要检查而已”，从而使这一次检查的用意更清晰。当然，合并前和合并后的代码有着相同的效果，但原先代码传达出的信息却是“这里有一些各自独立的条件测试，它们只是恰好同时发生”。其次，这项重构往往可以为你使用Extract Method (110)做好准备。将检查条件提炼成一个独立函数对于厘清代码意义非常有用，因为它把描述“做什么”的语句换成了“为什么这样做”。

条件语句的合并理由也同时指出了不要合并的理由：如果你认为这些检查的确彼此独立，的确不应该被视为同一次检查，那么就不要使用本项重构。因为在这种情况下，你的代码已经清楚表达出自己的意义。

做法

确定这些条件语句都没有副作用。

如果条件表达式有副作用，你就不能使用本项重构。

使用适当的逻辑操作符，将一系列相关条件表达式合并为一个。

编译，测试。

对合并后的条件表达式实施Extract Method (110)。

范例：使用逻辑或

请看下列代码：

double disabilityAmount(){

if (_seniority < 2)return 0;

if (_monthsDisabled > 12)return 0;

if (_isPartTime)return 0;

// compute the disability amount

...

在这段代码中，我们看到一连串的条件检查，它们都做同一件事。对于这样的代码，上述条件检查等价于一个以逻辑或连接起来的语句：

double disabilityAmount(){

if ((_seniority < 2)|| (_monthsDisabled > 12)|| (_isPartTime))return 0;

// compute the disability amount

...

现在，我可以观察这个新的条件表达式，并运用Extract Method (110)将它提炼成一个独立函数，以函数名称表达该语句所检查的条件：

double disabilityAmount(){

if (isNotEligibleForDisability())return 0;

// compute the disability amount

...

}

boolean isNotEligibleForDisability(){

return ((_seniority < 2)|| (_monthsDisabled > 12)|| (_isPartTime));

}

范例：使用逻辑与

上述实例展示了逻辑或的用法。下列代码展示逻辑与的用法：

if (onVacation())

if (lengthOfService()> 10)

return 1;

return 0.5;

这段代码可以变成：

if (onVacation()&& lengthOfService()> 10)return 1;

else return 0.5;

你可能还会发现，某些情况下需要同时使用逻辑或、逻辑与和逻辑非，最终得到的条件表达式可能很复杂，所以我会先使用Extract Method (110)将表达式的一部分提炼出来，从而使整个表达式变得简单一些。

如果我所观察的部分只是对条件进行检查并返回一个值，就可以使用三元操作符将这一部分变成一条return语句。因此，下列代码：

if (onVacation()&& lengthOfService()> 10)return 1;

else return 0.5;

就变成了：

return (onVacation()&& lengthOfService()> 10)? 1 : 0.5;

---

## 9.3 Consolidate Duplicate Conditional Fragments （合并重复的条件片段）

在条件表达式的每个分支上有着相同的一段代码。

将这段重复代码搬移到条件表达式之外。

if (isSpecialDeal()){
total = price * 0.95;
send();
}
else {
total = price * 0.98;
send();
}

if (isSpecialDeal())
total = price * 0.95;
else
total = price * 0.98;
send();

动机

有时你会发现，一组条件表达式的所有分支都执行了相同的某段代码。如果是这样，你就应该将这段代码搬移到条件表达式外面。这样，代码才能更清楚地表明哪些东西随条件的变化而变化、哪些东西保持不变。

做法

鉴别出“执行方式不随条件变化而变化”的代码。

如果这些共通代码位于条件表达式起始处，就将它移到条件表达式之前。

如果这些共通代码位于条件表达式尾端，就将它移到条件表达式之后。

如果这些共通代码位于条件表达式中段，就需要观察共通代码之前或之后的代码是否改变了什么东西。如果的确有所改变，应该首先将共通代码向前或向后移动，移至条件表达式的起始处或尾端，再以前面所说的办法来处理。

如果共通代码不止一条语句，应该首先使用Extract Method (110)将共通代码提炼到一个独立函数中，再以前面所说的办法来处理。

范例

你可能遇到这样的代码：

if (isSpecialDeal()){

total = price * 0.95;

send();

}

else {

total = price * 0.98;

send();

}

由于条件表达式的两个分支都执行了send()函数，所以我应该将send()移到条件表达式的外围：

if (isSpecialDeal())

total = price * 0.95;

else

total = price * 0.98;

send();

我们也可以使用同样的手法来对待异常。如果在try区段内可能引发异常的语句之后，以及所有catch区段之内，都重复执行了同一段代码，就可以将这段重复代码移到final区段。

---

## 9.4 Remove Control Flag（移除控制标记）

在一系列布尔表达式中，某个变量带有“控制标记”（control flag）的作用。

以break语句或return语句取代控制标记。

动机

在一系列条件表达式中，你常常会看到用以判断何时停止条件检查的控制标记：

set done to false

while not done

if (condition)

do something

set done to true

next step of loop

这样的控制标记带来的麻烦超过了它所带来的便利。人们之所以会使用这样的控制标记，因为结构化编程原则告诉他们：每个子程序只能有一个入口和一个出口。我赞同“单一入口”原则（而且现代编程语言也强迫我们这样做），但是“单一出口”原则会让你在代码中加入讨厌的控制标记，大大降低条件表达式的可读性。这就是编程语言提供break语句和continue语句的原因：用它们跳出复杂的条件语句。去掉控制标记所产生的效果往往让你大吃一惊：条件语句真正的用途会清晰得多。

做法

对控制标记的处理，最显而易见的办法就是使用Java提供的break语句或continue语句。

找出让你跳出这段逻辑的控制标记值。

找出对标记变量赋值的语句，代以恰当的break语句或continue语句。

每次替换后，编译并测试。

在未能提供break和continue语句的编程语言中，可以使用下述办法。

运用Extract Method (110)，将整段逻辑提炼到一个独立函数中。

找出让你跳出这段逻辑的控制标记值。

找出对标记变量赋值的语句，代以恰当的return语句。

每次替换后，编译并测试。

即使在支持break和continue语句的编程语言中，我通常也优先考虑上述第二种方案。因为return语句可以非常清楚地表示：不再执行该函数中的其他任何代码。如果还有这一类代码，你早晚需要将这段代码提炼出来。

请注意标记变量是否会影响这段逻辑的最后结果。如果有影响，使用break语句之后还得保留控制标记值。如果你已经将这段逻辑提炼成一个独立函数，也可以将控制标记值放在return语句中返回。

范例：以break取代简单的控制标记

下列函数用来检查一系列人名之中是否包含两个可疑人物的名字（这两个人的名字硬编码于代码中）：

void checkSecurity(String[] people){

boolean found = false;

for (int i = 0; i < people.length; i++){

if (!found){

if (people[i].equals("Don")){

sendAlert();

found = true;

}

if (people[i].equals("John")){

sendAlert();

found = true;

}

}

}

}

这种情况下很容易找出控制标记：当变量found被赋予true时，搜索就结束。我可以逐一引入break语句：

void checkSecurity(String[] people){

boolean found = false;

for (int i = 0; i < people.length; i++){

if (!found){

if (people[i].equals("Don")){

sendAlert();

break;

}

if (people[i].equals("John")){

sendAlert();

found = true;

}

}

}

}

直到替换掉所有对found变量赋值的语句：

void checkSecurity(String[] people){

boolean found = false;

for (int i = 0; i < people.length; i++){

if (!found){

if (people[i].equals("Don")){

sendAlert();

break;

}

if (people[i].equals("John")){

sendAlert();

break;

}

}

}

}

然后就可以把所有对控制标记的引用都去掉：

void checkSecurity(String[] people){

for (int i = 0; i < people.length; i++){

if (people[i].equals("Don")){

sendAlert();

break;

}

if (people[i].equals("John")){

sendAlert();

break;

}

}

}

范例：以return返回控制标记

本项重构的另一种形式将使用return语句。为了阐述这种用法，我把前面的例子稍加修改，以控制标记记录搜索结果：

void checkSecurity(String[] people){

String found = "";

for (int i = 0; i < people.length; i++){

if (found.equals("")){

if (people[i].equals("Don")){

sendAlert();

found = "Don";

}

if (people[i].equals("John")){

sendAlert();

found = "John";

}

}

}

someLaterCode(found);

}

在这里，变量found做了两件事：它既是控制标记，也是运算结果。遇到这种情况，我喜欢先把计算found变量的代码提炼到一个独立函数中：

void checkSecurity(String[] people){

String found = foundMiscreant(people);

someLaterCode(found);

}

String foundMiscreant(String[] people){

String found = "";

for (int i = 0; i < people.length; i++){

if (found.equals("")){

if (people[i].equals("Don")){

sendAlert();

found = "Don";

}

if (people[i].equals("John")){

sendAlert();

found = "John";

}

}

}

return found;

}

然后以return语句取代控制标记：

String foundMiscreant(String[] people){

String found = "";

for (int i = 0; i < people.length; i++){

if (found.equals("")){

if (people[i].equals("Don")){

sendAlert();

return "Don";

}

if (people[i].equals("John")){

sendAlert();

found = "John";

}

}

}

return found;

}

最后完全去掉控制标记：

String foundMiscreant(String[] people){

for (int i = 0; i < people.length; i++){

if (people[i].equals("Don")){

sendAlert();

return "Don";

}

if (people[i].equals("John")){

sendAlert();

return "John";

}

}

return "";

}

即使不需要返回某值，也可以用return语句来取代控制标记。这时候你只需要一个空的return语句就行了。

当然，如果以此办法去处理带有副作用的函数，会有一些问题。所以我需要先以Separate Query from Modifier (279)将函数副作用分离出去。稍后你会看到这方面的例子。

---

## 9.5 Replace Nested Conditional with Guard Clauses （以卫语句取代嵌套条件表达式）

函数中的条件逻辑使人难以看清正常的执行路径。

使用卫语句表现所有特殊情况。

double getPayAmount(){
double result;
if (_isDead)result = deadAmount();
else {
if (_isSeparated)result = separatedAmount();
else {
if (_isRetired)result = retiredAmount();
else result = normalPayAmount();
};
}
return result;
};

double getPayAmount(){
if (_isDead)return deadAmount();
if (_isSeparated)return separatedAmount();
if (_isRetired)return retiredAmount();
return normalPayAmount();
};

动机

根据我的经验，条件表达式通常有两种表现形式。第一种形式是：所有分支都属于正常行为。第二种形式则是：条件表达式提供的答案中只有一种是正常行为，其他都是不常见的情况。

这两类条件表达式有不同的用途，这一点应该通过代码表现出来。如果两条分支都是正常行为，就应该使用形如if...else...的条件表达式；如果某个条件极其罕见，就应该单独检查该条件，并在该条件为真时立刻从函数中返回。这样的单独检查常常被称为“卫语句”（guard clauses） [Beck]。

Replace Nested Conditional with Guard Clauses (250)的精髓就是：给某一条分支以特别的重视。如果使用if-then-else结构，你对if分支和else分支的重视是同等的。这样的代码结构传递给阅读者的消息就是：各个分支有同样的重要性。卫语句就不同了，它告诉阅读者：“这种情况很罕见，如果它真地发生了，请做一些必要的整理工作，然后退出。”

“每个函数只能有一个入口和一个出口”的观念，根深蒂固于某些程序员的脑海里。我发现，当我处理他们编写的代码时，经常需要使用Replace Nested Conditional with Guard Clauses (250)。现今的编程语言都会强制保证每个函数只有一个入口，至于“单一出口”规则，其实不是那么有用。在我看来，保持代码清晰才是最关键的：如果单一出口能使这个函数更清楚易读，那么就使用单一出口；否则就不必这么做。

做法

对于每个检查，放进一个卫语句。

卫语句要不就从函数中返回，要不就抛出一个异常。

每次将条件检查替换成卫语句后，编译并测试。

如果所有卫语句都导致相同结果，请使用Consolidate Conditional Expressions (240)。

范例

想象一个薪册系统，其中以特殊规则处理死亡员工、驻外员工、退休员工的薪资。这些情况不常有，但的确偶而会出现。

假设我在这个系统中看到下列代码：

double getPayAmount(){

double result;

if (_isDead)result = deadAmount();

else {

if (_isSeparated)result = separatedAmount();

else {

if (_isRetired)result = retiredAmount();

else result = normalPayAmount();

};

}

return result;

};

在这段代码中，非正常情况的检查掩盖了正常情况的检查，所以应该用卫语句来取代这些检查，以提高程序清晰度。我可以逐一引入卫语句。让我们从最上面的条件检查动作开始：

double getPayAmount(){

double result;

if (_isDead)return deadAmount();

if (_isSeparated)result = separatedAmount();

else {

if (_isRetired)result = retiredAmount();

else result = normalPayAmount();

};

return result;

};

然后，继续下去，仍然一次替换一个检查动作：

double getPayAmount(){

double result;

if (_isDead)return deadAmount();

if (_isSeparated)return separatedAmount();

if (_isRetired)result = retiredAmount();

else result = normalPayAmount();

return result;

};

然后是最后一个：

double getPayAmount(){

double result;

if (_isDead)return deadAmount();

if (_isSeparated)return separatedAmount();

if (_isRetired)return retiredAmount();

result = normalPayAmount();

return result;

};

此时，result变量已经没有价值了，所以我把它删掉：

double getPayAmount(){

if (_isDead)return deadAmount();

if (_isSeparated)return separatedAmount();

if (_isRetired)return retiredAmount();

return normalPayAmount();

};

嵌套条件代码往往由那些深信“每个函数只能有一个出口”的程序员写出。我发现那条规则实在有点太简单粗暴了。如果对函数剩余部分不再有兴趣，当然应该立刻退出。引导阅读者去看一个没有用的else区段，只会妨碍他们的理解。

范例：将条件反转

审阅本书初稿时，Joshua Kerievsky指出：你常常可以将条件表达式反转，从而实现Replace Nested Conditional with Guard Clauses (250)。为了拯救我可怜的想象力，他还好心帮我想了个例子：

public double getAdjustedCapital(){

double result = 0.0;

if (_capital > 0.0){

if (_intRate > 0.0 && _duration > 0.0){

result = (_income / _duration)* ADJ_FACTOR;

}

}

return result;

}

同样地，我逐一进行替换。不过这次在插入卫语句时，我需要将相应的条件反转过来：

public double getAdjustedCapital(){

double result = 0.0;

if (_capital <= 0.0)return result;

if (_intRate > 0.0 && _duration > 0.0){

result = (_income / _duration)* ADJ_FACTOR;

}

return result;

}

下一个条件稍微复杂一点，所以我分两步进行逆反。首先加入一个逻辑非操作：

public double getAdjustedCapital(){

double result = 0.0;

if (_capital <= 0.0)return result;

if (!(_intRate > 0.0 && _duration > 0.0))return result;

result = (_income / _duration)* ADJ_FACTOR;

return result;

}

但是在这样的条件表达式中留下一个逻辑非，会把我的脑袋拧成一团乱麻，所以我把它简化成下面这样：

public double getAdjustedCapital(){

double result = 0.0;

if (_capital <= 0.0)return result;

if (_intRate <= 0.0 || _duration <= 0.0)return result;

result = (_income / _duration)* ADJ_FACTOR;

return result;

}

这时候，我比较喜欢在卫语句内返回一个明确值，因为这样我可以一目了然地看到卫语句返回的失败结果。此外，这种时候我也会考虑使用Replace Magic Number with Symbolic Constant (204)。

public double getAdjustedCapital(){

double result = 0.0;

if (_capital <= 0.0)return 0.0;

if (_intRate <= 0.0 || _duration <= 0.0)return 0.0;

result = (_income / _duration)* ADJ_FACTOR;

return result;

}

完成替换之后，我同样可以将临时变量移除：

public double getAdjustedCapital(){

if (_capital <= 0.0)return 0.0;

if (_intRate <= 0.0 || _duration <= 0.0)return 0.0;

return (_income / _duration)* ADJ_FACTOR;

}

---

## 9.6 Replace Conditional with Polymorphism （以多态取代条件表达式）

你手上有个条件表达式，它根据对象类型的不同而选择不同的行为。

将这个条件表达式的每个分支放进一个子类内的覆写函数中，然后将原始函数声明为抽象函数。

double getSpeed(){
switch (_type){
case EUROPEAN:
return getBaseSpeed();
case AFRICAN:
return getBaseSpeed()- getLoadFactor()* _numberOfCoconuts;
case NORWEGIAN_BLUE:
return (_isNailed)? 0 : getBaseSpeed(_voltage);
}
throw new RuntimeException("Should be unreachable");
}

动机

在面向对象术语中，听上去最高贵的词非“多态”莫属。多态最根本的好处就是：如果你需要根据对象的不同类型而采取不同的行为，多态使你不必编写明显的条件表达式。

正因为有了多态，所以你会发现：“类型码的switch语句”以及“基于类型名称的if-then-else语句”在面向对象程序中很少出现。

多态能够给你带来很多好处。如果同一组条件表达式在程序许多地点出现，那么使用多态的收益是最大的。使用条件表达式时，如果你想添加一种新类型，就必须查找并更新所有条件表达式。但如果改用多态，只需建立一个新的子类，并在其中提供适当的函数就行了。类的用户不需要了解这个子类，这就大大降低了系统各部分之间的依赖，使系统升级更加容易。

做法

使用Replace Conditional with Polymorphism (255)之前，首先必须有一个继承结构。你可能已经通过先前的重构得到了这一结构。如果还没有，现在就需要建立它。

要建立继承结构，有两种选择：Replace Type Code with Subclasses (223)和Replace Type Code with State/Strategy (227)。前一种做法比较简单，因此应该尽可能使用它。但如果你需要在对象创建好之后修改类型码，就不能使用继承手法，只能使用State/Strategy模式。此外，如果由于其他原因，要重构的类已经有了子类，那么也得使用State/Strategy。记住，如果若干switch语句针对的是同一个类型码，你只需针对这个类型码建立一个继承结构就行了。

现在，可以向条件表达式开战了。你的目标可能是switch语句，也可能是if语句。

如果要处理的条件表达式是一个更大函数中的一部分，首先对条件表达式进行分析，然后使用Extract Method (110)将它提炼到一个独立函数去。

如果有必要，使用Move Method (142)将条件表达式放置到继承结构的顶端。

任选一个子类，在其中建立一个函数，使之覆写超类中容纳条件表达式的那个函数。将与该子类相关的条件表达式分支复制到新建函数中，并对它进行适当调整。

为了顺利进行这一步骤，你可能需要将超类中的某些private字段声明为protected。

编译，测试。

在超类中删掉条件表达式内被复制了的分支。

编译，测试。

针对条件表达式的每个分支，重复上述过程，直到所有分支都被移到子类内的函数为止。

将超类之中容纳条件表达式的函数声明为抽象函数。

范例

请允许我继续使用“员工与薪资”这个简单而又乏味的例子。我的类是从Replace Type Code with State/Strategy (227)那个例子中拿来的，因此示意图就如图9-1所示（如果想知道这个图是怎么得到的，请看第8章的范例）。

图9-1 继承结构

class Employee...

int payAmount(){

switch (getType()){

case EmployeeType.ENGINEER:

return _monthlySalary;

case EmployeeType.SALESMAN:

return _monthlySalary + _commission;

case EmployeeType.MANAGER:

return _monthlySalary + _bonus;

default:

throw new RuntimeException("Incorrect Employee");

}

}

int getType(){

return _type.getTypeCode();

}

private EmployeeType _type;

abstract class EmployeeType...

abstract int getTypeCode();

class Engineer extends EmployeeType...

int getTypeCode(){

return Employee.ENGINEER;

}

...and other subclasses

switch语句已经被很好地提炼出来，因此我不必费劲再做一遍。不过我需要将它移到EmployeeType类，因为EmployeeType才是要被继承的类。

class EmployeeType...

int payAmount(Employee emp){

switch (getTypeCode()){

case ENGINEER:

return emp.getMonthlySalary();

case SALESMAN:

return emp.getMonthlySalary()+ emp.getCommission();

case MANAGER:

return emp.getMonthlySalary()+ emp.getBonus();

default:

throw new RuntimeException("Incorrect Employee");

}

}

由于我需要Employee的数据，所以需要将Employee对象作为参数传递给payAmount()。这些数据中的一部分也许可以移到EmployeeType来，但那是另一项重构需要关心的问题了。

调整代码，使之通过编译，然后我修改Employee中的payAmount()函数，令它委托EmployeeType：

class Employee...

int payAmount(){

return _type.payAmount(this);

}

现在，我可以处理switch语句了。这个过程有点像淘气小男孩折磨一只昆虫——每次掰掉它一条腿。首先我把switch语句中的Engineer这一分支复制到Engineer类：

class Engineer...

int payAmount(Employee emp){

return emp.getMonthlySalary();

}

这个新函数覆写了超类中的switch语句内专门处理Engineer的分支。我是个偏执狂，有时我会故意在case子句中放一个陷阱，检查Engineer子类是否正常工作：

class EmployeeType...

int payAmount(Employee emp){

switch (getTypeCode()){

case ENGINEER:

throw new RuntimeException("Should be being overridden");

case SALESMAN:

return emp.getMonthlySalary()+ emp.getCommission();

case MANAGER:

return emp.getMonthlySalary()+ emp.getBonus();

default:

throw new RuntimeException("Incorrect Employee");

}

}

接下来，我重复上述过程，直到所有分支都被去除为止：

class Salesman...

int payAmount(Employee emp){

return emp.getMonthlySalary()+ emp.getCommission();

}

class Manager...

int payAmount(Employee emp){

return emp.getMonthlySalary()+ emp.getBonus();

}

然后，将超类的payAmount()函数声明为抽象函数：

class EmployeeType...

abstract int payAmount(Employee emp);

---

## 9.7 Introduce Null Object（引入Null对象）

你需要再三检查某对象是否为null。

将null值替换为null对象。

if (customer == null)plan = BillingPlan.basic();
else plan = customer.getPlan();

动机

多态的最根本好处在于：你不必再向对象询问“你是什么类型”而后根据得到的答案调用对象的某个行为——你只管调用该行为就是了，其他的一切多态机制会为你安排妥当。当某个字段内容是null 时，多态可扮演另一个较不直观（亦较不为人所知）的用途。让我们先听听Ron Jeffries的故事。

——Ron Jeffries

我们第一次使用Null Object模式，是因为Rich Garzaniti发现，系统在向对象发送一个消息之前，总要检查对象是否存在，这样的检查出现很多次。我们可能会向一个对象索求它所相关的Person对象，然后再问那个对象是否为null。如果对象的确存在，我们才能调用它的rate()函数以查询这个人的薪资级别。我们在好些地方都是这样做的，造成的重复代码让我们很烦心。

所以，我们编写了一个MissingPerson类，让它返回‘0’薪资等级[我们也把空对象（null object）称为虚拟对象（missing object）]。很快地，MissingPerson就有了很多函数，rate()自然是其中之一。如今我们的系统有超过80个空对象类。

我们常常在显示信息的时候使用空对象。例如我们想要显示一个Person对象信息，它大约有20个实例变量。如果这些变量可被设为null，那么打印一个Person对象的工作将非常复杂。所以我们不让实例变量被设为null，而是插入各式各样的空对象——它们都知道如何正确地显示自己。这样，我们就可以摆脱大量过程化的代码。

我们对空对象的最聪明运用，就是拿它来表示不存在的Gemstone会话：我们使用Gemstone数据库来保存成品（程序代码），但我们更愿意在没有数据库的情况下进行开发，每过一周左右再把新代码放进Gemstone数据库。然而在代码的某些地方，我们必须登录一个Gemstone会话。当没有Gemstone数据库时，我们就仅仅安插一个“虚构的Gemstone会话”，其接口和真正的Gemstone会话一模一样，使我们无需判断数据库是否存在，就可以进行开发和测试。

空对象的另一个用途是表现出“虚构的箱仓”（missing bin）。所谓“箱仓”，这里是指集合，用来保存某些薪资值，并常常需要对各个薪资值进行加和或遍历。如果某个箱仓不存在，我们就给出一个虚构的箱仓对象，其行为和一个空箱仓一样。这个虚构箱仓知道自己其实不带任何数据，总值为0。通过这种做法，我们就不必为上千位员工每人产生数十来个空箱对象了。

使用空对象时有个非常有趣的性质：系统几乎从来不会因为空对象而被破坏。由于空对象对所有外界请求的响应都和真实对象一样，所以系统行为总是正常的。但这并非总是好事，有时会造成问题的侦测和查找上的困难，因为从来没有任何东西被破坏。当然，只要认真检查一下，你就会发现空对象有时出现在不该出现的地方。

请记住：空对象一定是常量，它们的任何成分都不会发生变化。因此我们可以使用Singleton模式[Gang of Four]来实现它们。例如不管任何时候，只要你索求一个MissingPerson对象，得到的一定是MissingPerson 的唯一实例。

关于Null Object模式，你可以在Woolf [Woolf]中找到更详细的介绍。

做法

为源类建立一个子类，使其行为就像是源类的null版本。在源类和null子类中都加上isNull()函数，前者的isNull()应该返回false，后者的isNull()应该返回true。

下面这个办法也可能对你有所帮助：建立一个nullable接口，将isNull()函数放在其中，让源类实现这个接口。

另外，你也可以创建一个测试接口，专门用来检查对象是否为null。

编译。

找出所有“索求源对象却获得一个null”的地方。修改这些地方，使它们改而获得一个空对象。

找出所有“将源对象与null做比较”的地方。修改这些地方，使它们调用isNull()函数。

你可以每次只处理一个源对象及其客户程序，编译并测试后，再处理另一个源对象。

你可以在“不该再出现null”的地方放上一些断言，确保null的确不再出现。这可能对你有所帮助。

编译，测试。

找出这样的程序点：如果对象不是null，做A动作，否则做B动作。

对于每一个上述地点，在null类中覆写A动作，使其行为和B动作相同。

使用上述被覆写的动作，然后删除“对象是否等于null”的条件测试。编译并测试。

范例

一家公用事业公司的系统以Site表示地点（场所）。庭院宅第（house）和集体公寓（apartment）都使用该公司的服务。任何时候每个地点都拥有（或说都对应于）一个顾客，顾客信息以Customer表示：

class Site...

Customer getCustomer(){

return _customer;

}

Customer _customer;

Customer有很多特性，我们只看其中三项：

class Customer...

public String getName(){...}

public BillingPlan getPlan(){...}

public PaymentHistory getHistory(){...}

本系统又以PaymentHistory表示顾客的付款记录，它也有其自己的特性：

public class PaymentHistory...

int getWeeksDelinquentInLastYear()

上面的各种取值函数允许客户取得各种数据。但有时候一个地点的顾客搬走了，新顾客还没搬进来，此时这个地点就没有顾客。由于这种情况有可能发生，所以我们必须保证Customer的所有用户都能够处理“Customer对象等于null”的情况。下面是一些示例片段：

Customer customer = site.getCustomer();

BillingPlan plan;

if (customer == null)plan = BillingPlan.basic();

else plan = customer.getPlan();

...

String customerName;

if (customer == null)customerName = "occupant";

else customerName = customer.getName();

...

int weeksDelinquent;

if (customer == null)weeksDelinquent = 0;

else weeksDelinquent = customer.getHistory().getWeeksDelinquentInLastYear();

这个系统中可能有许多地方使用Site和Customer对象，它们都必须检查Customer对象是否等于null，而这样的检查完全是重复的。看来是使用空对象的时候了。

首先新建一个NullCustomer，并修改Customer，使其支持“对象是否为null”的检查：

class NullCustomer extends Customer {

public boolean isNull(){

return true;

}

}

class Customer...

public boolean isNull(){

return false;

}

protected Customer(){} //needed by the NullCustomer

如果你无法修改Customer，可以使用第266页的做法：建立一个新的测试接口。

如果你喜欢，也可以新建一个接口，昭告大家“这里使用了空对象”：

interface Nullable {

boolean isNull();

}

class Customer implements Nullable

我还喜欢加入一个工厂函数，专门用来创建NullCtomer对象。这样一来，用户就不必知道空对象的存在了：

class Customer...

static Customer newNull(){

return new NullCustomer();

}

接下来的部分稍微有点麻烦。对于所有“返回null”的地方，我都要将它改为“返回空对象”。此外，我还要把foo==null这样的检查替换成foo.isNull()。我发现下列办法很有用：查找所有提供Customer对象的地方，将它们都加以修改，使它们不能返回null，改而返回一个NullCustomer对象。

class Site...

Customer getCustomer(){

return (_customer == null)?

Customer.newNull():

_customer;

}

另外，我还要修改所有使用Customer对象的地方，让它们以isNull()函数进行检查，不再使用==null检查方式。

Customer customer = site.getCustomer();

BillingPlan plan;

if (customer.isNull())plan = BillingPlan.basic();

else plan = customer.getPlan();

...

String customerName;

if (customer.isNull())customerName = "occupant";

else customerName = customer.getName();

...

int weeksDelinquent;

if (customer.isNull())weeksDelinquent = 0;

else weeksDelinquent = customer.getHistory().getWeeksDelinquentInLastYear();

毫无疑问，这是本项重构中最需要技巧的部分。对于每一个需要替换的可能等于null的对象，我都必须找到所有检查它是否等于null的地方，并逐一替换。如果这个对象被传播到很多地方，追踪起来就很困难。上述范例中，我必须找出每一个类型为Customer的变量，以及它们被使用的地点。很难将这个过程分成更小的步骤。有时候我发现可能等于null的对象只在某几处被用到，那么替换工作比较简单；但是大多数时候我必须做大量替换工作。还好，撤销这些替换并不困难，因为我可以不太困难地找出对isNull()的调用动作，但这毕竟也是很零乱很恼人的。

这个步骤完成之后，如果编译和测试都顺利通过，我就可以宽心地露出笑容了。接下来的动作比较有趣。到目前为止，使用isNull()函数尚未带来任何好处。只有把相关行为移到NullCustomer中并去除条件表达式之后，我才能得到切实的利益。我可以逐一将各种行为移过去。首先从“取得顾客名称”这个函数开始。此时的客户端代码大约如下：

String customerName;

if (customer.isNull())customerName = "occupant";

else customerName = customer.getName();

首先为NullCustomer加入一个合适的函数，通过这个函数来取得顾客名称：

class NullCustomer...

public String getName(){

return "occupant";

}}

现在，我可以去掉条件代码了：

String customerName = customer.getName();

接下来我以相同手法处理其他函数，使它们对相应查询做出合适的响应。此外我还可以对修改函数做适当的处理。于是下面这样的客户端程序：

if (! customer.isNull())

customer.setPlan(BillingPlan.special());

就变成了这样：

customer.setPlan(BillingPlan.special());

class NullCustomer...

public void setPlan (BillingPlan arg){}

请记住：只有当大多数客户代码都要求空对象做出相同响应时，这样的行为搬移才有意义。注意，我说的是“大多数”而不是“所有”。任何用户如果需要空对象做出不同响应，他们仍然可以使用isNull()函数来测试。只要大多数客户端都要求空对象做出相同响应，他们就可以调用默认的null行为，而你也就受益匪浅了。

上述范例略带差异的某种情况是，某些客户端使用Customer函数的运算结果：

if (customer.isNull())weeksDelinquent = 0;

else weeksDelinquent = customer.getHistory().getWeeksDelinquentInLastYear();

我可以新建一个NullPaymentHistory类，用以处理这种情况：

class NullPaymentHistory extends PaymentHistory...

int getWeeksDelinquentInLastYear(){

return 0;

}

并修改NullCustomer，让它返回一个NullPaymentHistory对象：

class NullCustomer...

public PaymentHistory getHistory(){

return PaymentHistory.newNull();

}

然后，我同样可以删除这一行条件代码：

int weeksDelinquent = customer.getHistory().getWeeksDelinquentInLastYear();

你常常可以看到这样的情况：空对象会返回其他空对象。

范例：测试接口

除了定义isNull()之外，你也可以建立一个用以检查“对象是否为null”的接口。

使用这种办法，需要新建一个Null接口，其中不定义任何函数：

interface Null {}

然后，让空对象实现Null接口：

class NullCustomer extends Customer implements Null...

然后，我就可以用instanceof操作符检查对象是否为null：

aCustomer instanceof Null

通常我尽量避免使用instanceof操作符，但在这种情况下，使用它是没问题的。而且这种做法还有另一个好处：不需要修改Customer。这么一来即使无法修改Customer源码，我也可以使用空对象。

其他特殊情况

使用本项重构时，你可以有几种不同的空对象，例如你可以说“没有顾客”（新建的房子和暂时没人住的房子）和“不知名顾客”（有人住，但我们不知道是谁）这两种情况是不同的。果真如此，你可以针对不同的情况建立不同的空对象类。有时候空对象也可以携带数据，例如不知名顾客的使用记录等，于是我们可以在查出顾客姓名之后将账单寄给他。

本质上来说，这是一个比Null Object模式更大的模式：Special Case模式。所谓特例类（special case），也就是某个类的特殊情况，有着特殊的行为。因此表示“不知名顾客”的UnknownCustomer和表示“没有顾客”的NoCustomer都是Customer的特例。你经常可以在表示数量的类中看到这样的“特例类”，例如Java浮点数有“正无穷大”、“负无穷大”和“非数量”（NaN）等特例。特例类的价值是：它们可以降低你的“错误处理”开销，例如浮点运算决不会抛出异常。如果你对NaN做浮点运算，结果也会是个NaN。这和“空对象的访问函数通常返回另一个空对象”是一样的道理。

---

## 9.8 Introduce Assertion（引入断言）

某一段代码需要对程序状态做出某种假设。

以断言明确表现这种假设。

double getExpenseLimit(){
// should have either expense limit or a primary project
return (_expenseLimit != NULL_EXPENSE)?
_expenseLimit:
_primaryProject.getMemberExpenseLimit();
}

double getExpenseLimit(){
Assert.isTrue (_expenseLimit != NULL_EXPENSE || _primaryProject != null);
return (_expenseLimit != NULL_EXPENSE)?
_expenseLimit:
_primaryProject.getMemberExpenseLimit();
}

动机

常常会有这样一段代码：只有当某个条件为真时，该段代码才能正常运行。例如平方根计算只对正值才能进行，又例如某个对象可能假设其字段至少有一个不等于null。

这样的假设通常并没有在代码中明确表现出来，你必须阅读整个算法才能看出。有时程序员会以注释写出这样的假设。而我要介绍的是一种更好的技术：使用断言明确标明这些假设。

断言是一个条件表达式，应该总是为真。如果它失败，表示程序员犯了错误。因此断言的失败应该导致一个非受控异常（unchecked exception）。断言绝对不能被系统的其他部分使用。实际上，程序最后的成品往往将断言统统删除。因此，标记“某些东西是个断言”是很重要的。

断言可以作为交流与调试的辅助。在交流的角度上，断言可以帮助程序阅读者理解代码所做的假设；在调试的角度上，断言可以在距离bug最近的地方抓住它们。当我编写自我测试代码的时候发现，断言在调试方面的帮助变得不那么重要了，但我仍然非常看重它们在交流方面的价值。

做法

如果程序员不犯错，断言就应该不会对系统运行造成任何影响，所以加入断言永远不会影响程序的行为。

如果你发现代码假设某个条件始终为真，就加入一个断言明确说明这种情况。

你可以新建一个Assert类，用于处理各种情况下的断言。

注意，不要滥用断言。请不要使用它来检查“你认为应该为真”的条件，请只使用它来检查“一定必须为真”的条件。滥用断言可能会造成难以维护的重复逻辑。在一段逻辑中加入断言是有好处的，因为它迫使你重新考虑这段代码的约束条件。如果不满足这些约束条件，程序也可以正常运行，断言就不会带给你任何帮助，只会把代码变得混乱，并且有可能妨碍以后的修改。

你应该常常问自己：如果断言所指示的约束条件不能满足，代码是否仍能正常运行？如果可以，就把断言拿掉。

另外，还需要注意断言中的重复代码。它们和其他任何地方的重复代码一样不好闻。你可以大胆使用Extract Method (110)去掉那些重复代码。

范例

下面是一个简单例子：开支限制。后勤部门的员工每个月有固定的开支限额；业务部门的员工则按照项目的开支限额来控制自己的开支。一个员工可能没有开支额度可用，也可能没有参与项目，但两者总要有一个（否则就没有经费可用了）。在开支限额相关程序中，上述假设总是成立的，因此：

class Employee...

private static final double NULL_EXPENSE = -1.0;

private double _expenseLimit = NULL_EXPENSE;

private Project _primaryProject;

double getExpenseLimit(){

return (_expenseLimit != NULL_EXPENSE)?

_expenseLimit:

_primaryProject.getMemberExpenseLimit();

}

boolean withinLimit(double expenseAmount){

return (expenseAmount <= getExpenseLimit());

}

这段代码包含了一个明显假设：任何员工要么参与某个项目，要么有个人开支限额。我们可以使用断言在代码中更明确地指出这一点：

double getExpenseLimit(){

Assert.isTrue(_expenseLimit != NULL_EXPENSE || _primaryProject != null);

return (_expenseLimit != NULL_EXPENSE)?

_expenseLimit :

_primaryProject.getMemberExpenseLimit();

}

这条断言不会改变程序的任何行为。另外，如果断言中的条件不为真，我就会收到一个运行期异常：也许是在withinLimit()函数中抛出一个空指针异常，也许是在Assert.isTrue()函数中抛出一个运行期异常。有时断言可以帮助程序员找到bug，因为它离出错地点很近。但是，更多时候，断言的价值在于：帮助程序员理解代码正确运行的必要条件。

我常对断言中的条件表达式使用Extract Method (110)，也许是为了将若干地方的重复码提炼到同一个函数中，也许只是为了更清楚说明条件表达式的用途。

在Java中使用断言有点麻烦：没有一种简单机制可以协助我们插入这东西
[1]

 。断言可被轻松拿掉，所以它们不可能影响最终成品的性能。编写一个辅助类（例如Assert类）当然有所帮助，可惜的是断言参数中的任何表达式不论什么情况都一定会被执行一遍。阻止它的唯一办法就是使用类似下面的手法：

double getExpenseLimit(){

Assert.isTrue(Assert.ON &&

(_expenseLimit != NULL_EXPENSE || _primaryProject != null));

return (_expenseLimit != NULL_EXPENSE)?

_expenseLimit

: _primaryProject.getMemberExpenseLimit();

}

或者是这种手法：

double getExpenseLimit(){

if (Assert.ON)

Assert.isTrue(_expenseLimit != NULL_EXPENSE ||

_primaryProject != null);

return (_expenseLimit != NULL_EXPENSE)?

_expenseLimit

: _primaryProject.getMemberExpenseLimit();

}

如果Assert.ON是个常量，编译器就会对它进行检查；如果它等于false，就不再执行条件表达式后半段代码。但是，加上这条语句实在有点丑陋，所以很多程序员宁可仅仅使用Assert.isTrue()函数，然后在项目结束前以过滤程序滤掉使用断言的每一行代码（可以使用Perl之类的语言来编写这样的过滤程序）。

Assert类应该有多个函数，函数名称应该帮助程序员理解其功用。除了isTrue()之外，你还可以为它加上equals()和shouldNeverReachHere()等函数。

[1].
 J2SE l.4已经支持断言语句。——译者注

---


# 第10章 简化函数调用 (Making Method Calls Simpler)

## 10.1 Rename Method（函数改名）

函数的名称未能揭示函数的用途。

修改函数名称。

动机

我极力提倡的一种编程风格就是：将复杂的处理过程分解成小函数。但是，如果做得不好，这会使你费尽周折却弄不清楚这些小函数各自的用途。要避免这种麻烦，关键就在于给函数起一个好名称。函数的名称应该准确表达它的用途。给函数命名有一个好办法：首先考虑应该给这个函数写上一句怎样的注释，然后想办法将注释变成函数名称。

生活就是如此。你常常无法第一次就给函数起一个好名称。这时候你可能会想：就这样将就着吧，毕竟只是一个名称而已。当心！这是恶魔的召唤，是通向混乱之路，千万不要被它诱惑！如果你看到一个函数名称不能很好地表达它的用途，应该马上加以修改。记住，你的代码首先是为人写的，其次才是为计算机写的。而人需要良好名称的函数。想想过去曾经浪费的无数时间吧。如果给每个函数都起一个良好的名称，也许你可以节约好多时间。起一个好名称并不容易，需要经验；要想成为一个真正的编程高手，起名的水平是至关重要的。当然，函数签名中的其他部分也一样重要。如果重新安排参数顺序，能够帮助提高代码的清晰度，那就大胆地去做吧，你有Add Parameter (275)和Remove Parameter (277)这两项武器。

做法

检查函数签名是否被超类或子类实现过。如果是，则需要针对每份实现分别进行下列步骤。

声明一个新函数，将它命名为你想要的新名称。将旧函数的代码复制到新函数中，并进行适当调整。

编译。

修改旧函数，令它将调用转发给新函数。

如果只有少数几个地方引用旧函数，你可以大胆地跳过这一步骤。

编译，测试。

找出旧函数的所有被引用点，修改它们，令它们改而引用新函数。每次修改后，编译并测试。

删除旧函数。

如果旧函数是该类public接口的一部分，你可能无法安全地删除它。这种情况下，将它保留在原处，并将它标记为deprecated（建议不使用）。

编译，测试。

范例

我以getTelephoneNumber()函数来取得某人的电话号码：

public String getTelephoneNumber(){

return ("(" + _officeAreaCode + ")" + _officeNumber);

}

现在，我想把这个函数改名为getOfficeTelephoneNumber()。首先建立一个新函数，命名为getOfficeTelephoneNumber()，并将原函数getTelephone-Number()的代码复制过来。然后，让旧函数直接调用新函数：

class Person...

public String getTelephoneNumber(){

return getOfficeTelephoneNumber();

}

public String getOfficeTelephoneNumber(){

return ("(" + _officeAreaCode + ")" + _officeNumber);

}

现在，我需要找到旧函数的所有调用者，将它们全部改为调用新函数。全部修改完后，就可以将旧函数删掉了。

如果需要添加或去除某个参数，过程也大致相同。

如果旧函数的调用者并不多，我可以直接修改这些调用者，令它们调用新函数，不必让旧函数充当中介。如果测试出错，我可以回到起始处，并放慢前进速度。

---

## 10.2 Add Parameter（添加参数）

某个函数需要从调用端得到更多信息。

为此函数添加一个对象参数，让该对象带进函数所需信息。

动机

Add Parameter (275)是一个很常用的重构手法，我几乎可以肯定你已经用过它了。使用这项重构的动机很简单：你必须修改一个函数，而修改后的函数需要一些过去没有的信息，因此你需要给该函数添加一个参数。

实际上我比较需要说明的是：不使用本重构的时机。除了添加参数外，你常常还有其他选择。只要可能，其他选择都比添加参数要好，因为它们不会增加参数列的长度。过长的参数列是不好的味道，因为程序员很难记住那么多参数，而且长参数列往往伴随着坏味道Data Clumps。

请看看现有的参数，然后问自己：你能从这些参数得到所需的信息吗？如果回答是否定的，有可能通过某个函数提供所需信息吗？你究竟把这些信息用于何处？这个函数是否应该属于拥有该信息的那个对象所有？看看现有参数，考虑一下，加入新参数是否合适？也许你应该考虑使用Introduce Parameter Object (295)。

我并非要你绝对不要添加参数。事实上我自己经常添加参数，但是在添加参数之前你有必要了解其他选择。

做法

Add Parameter (275)的做法和Rename Method (273)非常相似。

检查函数签名是否被超类或子类实现过。如果是，则需要针对每份实现分别进行下列步骤。

声明一个新函数，名称与原函数同，只是加上新添参数。将旧函数的代码复制到新函数中。

如果需要添加的参数不止一个，将它们一次性添加进去比较容易。

编译。

修改旧函数，令它调用新函数。

如果只有少数几个地方引用旧函数，你大可放心地跳过这一步骤。

此时，你可以给参数提供任意值。但一般来说，我们会给对象参数提供null，给内置型参数提供一个明显非正常值。对于数值型参数，我建议使用0以外的值，这样你比较容易将来认出它。

编译，测试。

找出旧函数的所有被引用点，将它们全部修改为对新函数的引用。每次修改后，编译并测试。

删除旧函数。

如果旧函数是该类public接口的一部分，你可能无法安全地删除它。这种情况下，请将它保留在原地，并将它标示为deprecated（建议不使用）。

编译，测试。

---

## 10.3 Remove Parameter（移除参数）

函数本体不再需要某个参数。

将该参数去除。

动机

程序员可能经常添加参数，却往往不愿意去掉它们。他们打的如意算盘是：无论如何，多余的参数不会引起任何问题，而且以后还可能用上它。

这也是恶魔的诱惑，一定要把它从脑子里赶出去！参数代表着函数所需的信息，不同的参数值有不同的意义。函数调用者必须为每一个参数操心该传什么东西进去。如果你不去掉多余参数，就是让你的每一位用户多费一份心。是很不划算的，更何况“去除参数”是非常简单的一项重构。

但是，对于多态函数，情况有所不同。这种情况下，可能多态函数的另一份（或多份）实现会使用这个参数，此时你就不能去除它。你可以添加一个独立函数，在这些情况下使用，不过你应该先检查调用者如何使用这个函数，以决定是否值得这么做。如果某些调用者已经知道他们正在处理的是一个特定的子类，并且已经做了额外工作找出自己需要的参数，或已经利用对类体系的了解来避免取到null，那么就值得你建立一个新函数，去除那多余的参数。如果调用者不需要了解该函数所属的类，你也可以继续保持调用者无知而幸福的状态。

做法

Remove Parameter (277)的做法和Rename Method (273)、Add Parameter (275)非常相似。

检查函数签名是否被超类或子类实现过。如果是，则需要针对每份实现分别进行下列步骤。

声明一个新函数，名称与原函数同，只是去除不必要的参数。将旧函数的代码复制到新函数中。

如果需要去除的参数不止一个，将它们一次性去除比较容易。

编译。

修改旧函数，令它调用新函数。

如果只有少数几个地方引用旧函数，你大可放心地跳过这一步骤。

编译，测试。

找出旧函数的所有被引用点，将它们全部修改为对新函数的引用。每次修改后，编译并测试。

删除旧函数。

如果旧函数是该类public接口的一部分，你可能无法安全地删除它。这种情况下，将它保留在原处，并将它标记为deprecated（建议不使用）。

编译，测试。

由于添加和去除参数都很简单，所以我经常一次性地添加或去除多个的参数。

---

## 10.4 Separate Query from Modifier （将查询函数和修改函数分离）

某个函数既返回对象状态值，又修改对象状态。

建立两个不同的函数，其中一个负责查询，另一个负责修改。

动机

如果某个函数只是向你提供一个值，没有任何看得到的副作用，那么这是个很有价值的东西。你可以任意调用这个函数，也可以把调用动作搬到函数的其他地方。简而言之，需要操心的事情少多了。

明确表现出“有副作用”与“无副作用”两种函数之间的差异，是个很好的想法。下面是一条好规则：任何有返回值的函数，都不应该有看得到的副作用。有些程序员甚至将此作为一条必须遵守的规则[Meyer]。就像对待任何东西一样，我并不绝对遵守它，不过我总是尽量遵守，而它也回报我很好的效果。

如果你遇到一个“既有返回值又有副作用”的函数，就应该试着将查询动作从修改动作中分割出来。

你也许已经注意到了：我使用“看得到的副作用”这种说法。有一种常见的优化办法是：将查询所得结果缓存于某个字段中，这么一来后续的重复查询就可以大大加快速度。虽然这种做法改变了对象的状态，但这一修改是察觉不到的，因为不论如何查询，你总是获得相同结果[Meyer]。

做法

新建一个查询函数，令它返回的值与原函数相同。

观察原函数，看它返回什么东西。如果返回的是一个临时变量，找出临时变量的位置。

修改原函数，令它调用查询函数，并返回获得的结果。

原函数中的每个return句都应该像这样：return newQuery()，而不应该返回其他东西。

如果调用者将返回值赋给了一个临时变量，你应该能够去除这个临时变量。

编译，测试。

将调用原函数的代码改为调用查询函数。然后，在调用查询函数的那一行之前，加上对原函数的调用。每次修改后，编译并测试。

将原函数的返回值改为void，并删掉其中所有的return语句。

范例

有这样一个函数：一旦有人入侵安全系统，它会告诉我入侵者的名字，并发送一个警报。如果入侵者不止一个，也只发送一条警报：

String foundMiscreant(String[] people){

for (int i = 0; i < people.length; i++){

if (people[i].equals("Don")){

sendAlert();

return "Don";

}

if (people[i].equals("John")){

sendAlert();

return "John";

}

}

return "";

}

该函数被下列代码调用：

void checkSecurity(String[] people){

String found = foundMiscreant(people);

someLaterCode(found);

}

为了将查询动作和修改动作分开，我首先建立一个适当的查询函数，使其与修改函数返回相同的值，但不造成任何副作用：

String foundPerson(String[] people){

for (int i = 0; i < people.length; i++){

if (people[i].equals("Don")){

return "Don";

}

if (people[i].equals("John")){

return "John";

}

}

return "";

}

然后，我要逐一替换原函数内所有的return语句，改调用新建的查询函数。每次替换后，编译并测试。这一步完成之后，原函数如下所示：

String foundMiscreant(String[] people){

for (int i = 0; i < people.length; i++){

if (people[i].equals("Don")){

sendAlert();

return foundPerson(people);

}

if (people[i].equals("John")){

sendAlert();

return foundPerson(people);

}

}

return foundPerson(people);

}

现在，我要修改调用者，将原本的单一调用动作替换为两个调用：先调用修改函数，然后调用查询函数：

void checkSecurity(String[] people){

foundMiscreant(people);

String found = foundPerson(people);

someLaterCode(found);

}

所有调用都替换完毕后，我就可以将修改函数的返回值改为void：

void foundMiscreant(String[] people){

for (int i = 0; i < people.length; i++){

if (people[i].equals("Don")){

sendAlert();

return;

}

if (people[i].equals("John")){

sendAlert();

return;

}

}

}

现在，为原函数改个名称可能会更好一些：

void sendAlert(String[] people){

for (int i = 0; i < people.length; i++){

if (people[i].equals("Don")){

sendAlert();

return;

}

if (people[i].equals("John")){

sendAlert();

return;

}

}

}

当然，这种情况下，我得到了大量重复代码，因为修改函数之中使用了与查询函数相同的代码。现在我可以对修改函数实施Substitute Algorithm (139)，设法让它再简洁一些：

void sendAlert(String[] people){

if (!foundPerson(people).equals(""))

sendAlert();

}

并发问题

如果你在一个多线程系统中工作，肯定知道这样一个重要的惯用手法：在同一个动作中完成检查和赋值。这是否和Separate Query from Modifier (279)互相矛盾呢？我曾经和Doug Lea讨论过这个问题，并得出结论：两者并不矛盾，但你需要做一些额外工作。将查询动作和修改动作分开来仍然是很有价值的。但你需要保留第三个函数来同时做这两件事。这个“查询—修改”函数将调用各自独立的查询函数和修改函数，并被声明为synchronized。如果查询函数和修改函数未被声明为synchronized，那么你还应该将它们的可见范围限制在包级别或private级别。这样，你就可以拥有一个安全、同步的操作，它由两个较易理解的函数组成。这两个较低层函数也可以用于其他场合。

---

## 10.5 Parameterize Method（令函数携带参数）

若干函数做了类似的工作，但在函数本体中却包含了不同的值。

建立单一函数，以参数表达那些不同的值。

动机

你可能会发现这样的两个函数：它们做着类似的工作，但因少数几个值致使行为略有不同。在这种情况下，你可以将这些各自分离的函数统一起来，并通过参数来处理那些变化情况，用以简化问题。这样的修改可以去除重复的代码，并提高灵活性，因为你可以用这个参数处理更多的变化情况。

做法

新建一个带有参数的函数，使它可以替换先前所有的重复性函数。

编译。

将调用旧函数的代码改为调用新函数。

编译，测试。

对所有旧函数重复上述步骤，每次替换后，修改并测试。

也许你会发现，你无法用这种办法处理整个函数，但可以处理函数中的一部分代码。这种情况下，你应该首先将这部分代码提炼到一个独立函数中，然后再对那个提炼所得的函数使用Parameterize Method (283)。

范例

下面是一个最简单的例子：

class Employee {

void tenPercentRaise(){

salary *= 1.1;

}

void fivePercentRaise(){

salary *= 1.05;

}

这段代码可以替换如下：

void raise (double factor){

salary *= (1 + factor);

}

当然，这个例子实在太简单了，所有人都能做到。

下面是一个稍微复杂的例子：

protected Dollars baseCharge(){

double result = Math.min(lastUsage(),100)* 0.03;

if (lastUsage()> 100){

result += (Math.min(lastUsage(),200)- 100)* 0.05;

}

if (lastUsage()> 200){

result += (lastUsage()- 200)* 0.07;

}

return new Dollars(result);

}

上述代码可以替换如下：

protected Dollars baseCharge(){

double result = usageInRange(0,100)* 0.03;

result += usageInRange(100,200)* 0.05;

result += usageInRange(200,Integer.MAX_VALUE)* 0.07;

return new Dollars(result);

}

protected int usageInRange(int start,int end){

if (lastUsage()> start)return Math.min(lastUsage(),end)- start;

else return 0;

}

本项重构的要点在于：以“可将少量数值视为参数”为依据，找出带有重复性的代码。

---

## 10.6 Replace Parameter with Explicit Methods （以明确函数取代参数）

你有一个函数，其中完全取决于参数值而采取不同行为。

针对该参数的每一个可能值，建立一个独立函数。

void setValue(String name,int value){
if (name.equals("height")){
_height = value;
return;
}
if (name.equals("width")){
_width = value;
return;
}
Assert.shouldNeverReachHere();
}

void setHeight(int arg){
_height = arg;
}
void setWidth(int arg){
_width = arg;
}

动机

Replace Parameter with Explicit Methods (285)恰恰相反于Parameterize Method (283)。如果某个参数有多种可能的值，而函数内又以条件表达式检查这些参数值，并根据不同参数值做出不同的行为，那么就应该使用本项重构。调用者原本必须赋予参数适当的值，以决定该函数做出何种响应。现在，既然你提供了不同的函数给调用者使用，就可以避免出现条件表达式。此外你还可以获得编译期检查的好处，而且接口也更清楚。如果以参数值决定函数行为，那么函数用户不但需要观察该函数，而且还要判断参数值是否合法，而“合法的参数值”往往很少在文档中被清楚地提出。

就算不考虑编译期检查的好处，只是为了获得一个清晰的接口，也值得你执行本项重构。哪怕只是给一个内部的布尔变量赋值，相较之下，Switch.beOn()也比Switch.setState(true)要清楚得多。

但是，如果参数值不会对函数行为有太多影响，你就不应该使用Replace Parameter with Explicit Methods (285)。如果情况真是这样，而你也只需要通过参数为一个字段赋值，那么直接使用设值函数就行了。如果的确需要条件判断的行为，可考虑使用Replace Conditional with Polymorphism (255)。

做法

针对参数的每一种可能值，新建一个明确函数。

修改条件表达式的每个分支，使其调用合适的新函数。

修改每个分支后，编译并测试。

修改原函数的每一个被调用点，改而调用上述的某个合适的新函数。

编译，测试。

所有调用端都修改完毕后，删除原函数。

范例

下列代码中，我想根据不同的参数值，建立Employee之下不同的子类。以下代码往往是Replace Constructor with Factory Method (304)的施行成果：

static final int ENGINEER = 0;

static final int SALESMAN = 1;

static final int MANAGER = 2;

static Employee create(int type){

switch (type){

case ENGINEER:

return new Engineer();

case SALESMAN:

return new Salesman();

case MANAGER:

return new Manager();

default:

throw new IllegalArgumentException("Incorrect type code value");

}

}

由于这是一个工厂函数，我不能实施Replace Conditional with Polymorphism (255)，因为使用该函数时对象根本还没创建出来。由于可以预见到Employee不会有太多新的子类，所以我可以放心地为每个子类建立一个工厂函数，而不必担心工厂函数的数量会剧增。首先，我要根据参数值建立相应的新函数：

static Employee createEngineer(){

return new Engineer();

}

static Employee createSalesman(){

return new Salesman();

}

static Employee createManager(){

return new Manager();

}

然后把switch语句的各个分支替换为对新函数的调用：

static Employee create(int type){

switch (type){

case ENGINEER:

return Employee.createEngineer();

case SALESMAN:

return new Salesman();

case MANAGER:

return new Manager();

default:

throw new IllegalArgumentException("Incorrect type code value");

}

}

每修改一个分支，都需要编译并测试，直到所有分支修改完毕为止：

static Employee create(int type){

switch (type){

case ENGINEER:

return Employee.createEngineer();

case SALESMAN:

return Employee.createSalesman();

case MANAGER:

return Employee.createManager();

default:

throw new IllegalArgumentException("Incorrect type code value");

}

}

接下来，我把注意力转移到旧函数的调用端。我把诸如下面这样的代码：

Employee kent = Employee.create(ENGINEER)

替换为：

Employee kent = Employee.createEngineer()

修改完create()函数的所有调用者之后，就可以把create()函数删掉了。同时也可以把所有常量都删掉。

---

## 10.7 Preserve Whole Object（保持对象完整）

你从某个对象中取出若干值，将它们作为某一次函数调用时的参数。

改为传递整个对象。

int low = daysTempRange().getLow();
int high = daysTempRange().getHigh();
withinPlan = plan.withinRange(low,high);

withinPlan = plan.withinRange(daysTempRange());

动机

有时候，你会将来自同一对象的若干项数据作为参数，传递给某个函数。这样做的问题在于：万一将来被调用函数需要新的数据项，你就必须查找并修改对此函数的所有调用。如果你把这些数据所属的整个对象传给函数，可以避免这种尴尬的处境，因为被调用函数可以向那个参数对象请求任何它想要的信息。

除了可以使参数列更稳固之外，Preserve Whole Object (288)往往还能提高代码的可读性。过长的参数列很难使用，因为调用者和被调用者都必须记住这些参数的用途。此外，不使用完整对象也会造成重复代码，因为被调用函数无法利用完整对象中的函数来计算某些中间值。

不过事情总有两面。如果你传的是数值，被调用函数就只依赖于这些数值，而不依赖它们所属的对象。但如果你传递的是整个对象，被调用函数所在的对象就需要依赖参数对象。如果这会使你的依赖结构恶化，那么就不该使用Preserve Whole Object (288)。

我还听过另一种不使用Preserve Whole Object (288)的理由：如果被调用函数只需要参数对象的其中一项数值，那么只传递那个数值会更好。我并不认同这种观点，因为传递一项数值和传递一个对象，至少在代码清晰度上是等价的（当然对于按值传递的参数来说，性能上可能有所差异）。更重要的考量应该放在对象之间的依赖关系上。

如果被调用函数使用了来自另一个对象的很多项数据，这可能意味该函数实际上应该被定义在那些数据所属的对象中。所以，考虑Preserve Whole Object (288)的同时，你也应该考虑Move Method (142)。

运用本项重构之前，你可能还没有定义一个完整对象。那么就应该先使用Introduce Parameter Object (295)。

还有一种常见情况：调用者将自己的若干数据作为参数，传递给被调用函数。这种情况下，如果该对象有合适的取值函数，你可以使用this取代这些参数值，并且无需操心对象依赖问题。

做法

对你的目标函数新添一个参数项，用以代表原数据所在的完整对象。

编译，测试。

判断哪些参数可被包含在新添的完整对象中。

选择上述参数之一，将被调用函数中原来引用该参数的地方，改为调用新添参数对象的相应取值函数。

删除该项参数。

编译，测试。

针对所有可从完整对象中获得的参数，重复上述过程。

删除调用端中那些带有被删除参数的代码。

当然，如果调用端还在其他地方使用了这些参数，就不要删除它们。

编译，测试。

范例

在以下范例中，我以一个Room对象表示“房间”，它负责记录房间一天中的最高温度和最低温度。然后这个对象需要将实际的温度范围与预先规定的温度控制计划相比较，告诉客户当天温度是否符合计划要求：

class Room...

boolean withinPlan(HeatingPlan plan){

int low = daysTempRange().getLow();

int high = daysTempRange().getHigh();

return plan.withinRange(low,high);

}

class HeatingPlan...

boolean withinRange(int low,int high){

return (low >= _range.getLow()&& high <= _range.getHigh());

}

private TempRange _range;

其实我不必将TempRange对象的信息拆开来单独传递，只需将整个对象传递给withinPlan()函数即可。在这个简单的例子中，我可以一次性完成修改。如果相关的参数更多些，我也可以进行小步重构。首先，我为参数列添加新的参数项，用以传递完整的TempRange对象：

class HeatingPlan...

boolean withinRange(TempRange roomRange,int low,int high){

return (low >= _range.getLow()&& high <= _range.getHigh());

}

class Room...

boolean withinPlan(HeatingPlan plan){

int low = daysTempRange().getLow();

int high = daysTempRange().getHigh();

return plan.withinRange(daysTempRange(),low,high);

}

然后，我以TempRange对象提供的函数来替换low参数：

class HeatingPlan...

boolean withinRange(TempRange roomRange,int high){

return (roomRange.getLow()>= _range.getLow()&& high <= _range.getHigh());

}

class Room...

boolean withinPlan(HeatingPlan plan){

int low = daysTempRange().getLow();

int high = daysTempRange().getHigh();

return plan.withinRange(daysTempRange(),high);

}

重复上述步骤，直到把所有待处理参数项都去除为止：

class HeatingPlan...

boolean withinRange(TempRange roomRange){

return (roomRange.getLow()>= _range.getLow()&& roomRange.getHigh()

<= _range.getHigh());

}

class Room...

boolean withinPlan(HeatingPlan plan){

int low = daysTempRange().getLow();

int high = daysTempRange().getHigh();

return plan.withinRange(daysTempRange());

}

现在，我不再需要low和high这两个临时变量了：

class Room...

boolean withinPlan(HeatingPlan plan){

int low = daysTempRange().getLow();

int high = daysTempRange().getHigh();

return plan.withinRange(daysTempRange());

}

使用完整对象后不久，你就会发现，可以将某些函数移到TempRange对象中，使它更容易被使用，例如：

class HeatingPlan...

boolean withinRange(TempRange roomRange){

return (_range.includes(roomRange));

}

class TempRange...

boolean includes(TempRange arg){

return arg.getLow()>= this.getLow()&& arg.getHigh()<= this.getHigh();

}

---

## 10.8 Replace Parameter with Methods（以函数取代参数）

对象调用某个函数，并将所得结果作为参数，传递给另一个函数。

而接受该参数的函数本身也能够调用前一个函数。

让参数接受者去除该项参数，并直接调用前一个函数。

int basePrice = _quantity * _itemPrice;
discountLevel = getDiscountLevel();
double finalPrice = discountedPrice (basePrice,discountLevel);

int basePrice = _quantity * _itemPrice;
double finalPrice = discountedPrice (basePrice);

动机

如果函数可以通过其他途径获得参数值，那么它就不应该通过参数取得该值。过长的参数列会增加程序阅读者的理解难度，因此我们应该尽可能缩短参数列的长度。

缩减参数列的办法之一就是：看看参数接受端是否可以通过与调用端相同的计算来取得参数值。如果调用端通过其所属对象内部的另一个函数来计算参数，并在计算过程中未曾引用调用端的其他参数，那么你就应该可以将这个计算过程转移到被调用端内，从而去除该项参数。如果你所调用的函数隶属另一对象，而该对象拥有调用端所属对象的引用，前面所说的这些也同样适用。

但是，如果参数值的计算过程依赖于调用端的某个参数，那么你就无法去掉被调用端的参数，因为每一次调用动作中，该参数值都可能不同（当然，如果你能够运用Replace Parameter with Explicit Methods (285)将该参数替换为一个函数，又另当别论）。另外，如果参数接受端并没有参数发送端对象的引用，而你也不想加上这样一个引用，那么也无法去除参数。

有时候，参数的存在是为了将来的灵活性。这种情况下我仍然会把这种多余参数拿掉。是的，你应该只在必要关头才添加参数，预先添加的参数很可能并不是你所需要的。不过，对于这条规则，也有一个例外：如果修改接口会对整个程序造成非常痛苦的结果（例如需要很长时间来重新构建程序，或需要修改大量代码），那么可以考虑保留前人预先加入的参数。如果真是这样，你应该首先判断修改接口究竟会造成多严重的后果，然后考虑是否应该降低系统各部位之间的依赖，以减少修改接口所造成的影响。稳定的接口确实很好，但是被冻结在一个不良接口上也是有问题的。

做法

如果有必要，将参数的计算过程提炼到一个独立函数中。

将函数本体内引用该参数的地方改为调用新建的函数。

每次替换后，修改并测试。

全部替换完成后，使用Remove Parameter (277)将该参数去掉。

范例

以下代码用于计算定单折扣价格。虽然这么低的折扣不大可能出现在现实生活中，不过作为一个范例，我们暂不考虑这一点：

public double getPrice(){

int basePrice = _quantity * _itemPrice;

int discountLevel;

if (_quantity > 100)discountLevel = 2;

else discountLevel = 1;

double finalPrice = discountedPrice (basePrice,discountLevel);

return finalPrice;

}

private double discountedPrice (int basePrice,int discountLevel){

if (discountLevel == 2)return basePrice * 0.1;

else return basePrice * 0.05;

}

首先，我把计算折扣等级（discountLevel）的代码提炼成为一个独立的getDiscountLevel()函数：

public double getPrice(){

int basePrice = _quantity * _itemPrice;

int discountLevel = getDiscountLevel();

double finalPrice = discountedPrice (basePrice,discountLevel);

return finalPrice;

}

private int getDiscountLevel(){

if (_quantity > 100)return 2;

else return 1;

}

然后把discountedPrice()函数中对discountLevel参数的所有引用点，替换为对getDiscountLevel()函数的调用：

private double discountedPrice (int basePrice,int discountLevel){

if (getDiscountLevel()== 2)return basePrice * 0.1;

else return basePrice * 0.05;

}

此时我就可以使用Remove Parameter (277)去掉discountLevel参数了：

public double getPrice(){

int basePrice = _quantity * _itemPrice;

int discountLevel = getDiscountLevel();

double finalPrice = discountedPrice (basePrice);

return finalPrice;

}

private double discountedPrice (int basePrice){

if (getDiscountLevel()== 2)return basePrice * 0.1;

else return basePrice * 0.05;

}

接下来可以将discountLevel变量去除掉：

public double getPrice(){

int basePrice = _quantity * _itemPrice;

double finalPrice = discountedPrice (basePrice);

return finalPrice;

}

现在，可以去掉其他非必要的参数和相应的临时变量。最后获得以下代码：

public double getPrice(){

return discountedPrice ();

}

private double discountedPrice (){

if (getDiscountLevel()== 2)return getBasePrice()* 0.1;

else return getBasePrice()* 0.05;

}

private double getBasePrice(){

return _quantity * _itemPrice;

}

最后我还可以针对discountedPrice()函数使用Inline Method (117)：

private double getPrice (){

if (getDiscountLevel()== 2)return getBasePrice()* 0.1;

else return getBasePrice()* 0.05;

}

---

## 10.9 Introduce Parameter Object（引入参数对象）

某些参数总是很自然地同时出现。

以一个对象取代这些参数。

动机

你常会看到特定的一组参数总是一起被传递。可能有好几个函数都使用这一组参数，这些函数可能隶属同一个类，也可能隶属不同的类。这样一组参数就是所谓的Data Clumps（数据泥团），我们可以运用一个对象包装所有这些数据，再以该对象取代它们。哪怕只是为了把这些数据组织在一起，这样做也是值得的。本项重构的价值在于缩短参数列，而你知道，过长的参数列总是难以理解的。此外，新对象所定义的访问函数还可以使代码更具一致性，这又进一步降低了理解和修改代码的难度。

本项重构还可以带给你更多好处。当你把这些参数组织到一起之后，往往很快可以发现一些可被移至新建类的行为。通常，原本使用那些参数的函数对这一组参数会有一些共通的处理，如果将这些共通行为移到新对象中，你可以减少很多重复代码。

做法

新建一个类，用以表现你想替换的一组参数。将这个类设为不可变的。

编译。

针对使用该组参数的所有函数，实施Add Parameter (275)，传入上述新建类的实例对象，并将此参数值设为null。

如果你所修改的函数被其他很多函数调用，那么可以保留修改前的旧函数，并令它调用修改后的新函数。你可以先对旧函数进行重构，然后逐一修改调用端使其调用新函数，最后再将旧函数删除。

对于Data Clumps中的每一项（在此均为参数），从函数签名中移除之，并修改调用端和函数本体，令它们都改而通过新的参数对象取得该值。

每去除一个参数，编译并测试。

将原先的参数全部去除之后，观察有无适当函数可以运用Move Method (142)搬移到参数对象之中。

被搬移的可能是整个函数，也可能是函数中的一个段落。如果是后者，首先使用Extract Method (110)将该段落提炼为一个独立函数，再搬移这一新建函数。

范例

下面是一个“账目和账项”范例。表示“账项”的Entry实际上只是个简单的数据容器：

class Entry...

Entry(double value,Date chargeDate){

_value = value;

_chargeDate = chargeDate;

}

Date getDate(){

return _chargeDate;

}

double getValue(){

return _value;

}

private Date _chargeDate;

private double _value;

我关注的焦点是用以表示“账目”的Account，它保存了一组Entry对象，并有一个函数用来计算两个日期间的账项总量：

class Account...

double getFlowBetween(Date start,Date end){

double result = 0;

Enumeration e = _entries.elements();

while (e.hasMoreElements()){

Entry each = (Entry)e.nextElement();

if (each.getDate().equals(start)||

each.getDate().equals(end)||

(each.getDate().after(start)&& each.getDate().before(end)))

{

result += each.getValue();

}

}

return result;

}

private Vector _entries = new Vector();

client code...

double flow = anAccount.getFlowBetween(startDate,endDate);

我已经记不清有多少次看见代码用一对值来表示一个范围，例如表示日期范围的start和end、表示数值范围的upper和lower，等等。我知道为什么会发生这种情况，毕竟我自己也经常这样做。不过，自从学到Range模式[Fowler，AP]之后，我就尽量以“范围对象”取而代之。我的第一个步骤是声明一个简单的数据容器，用以表示范围：

class DateRange {

DateRange(Date start,Date end){

_start = start;

_end = end;

}

Date getStart(){

return _start;

}

Date getEnd(){

return _end;

}

private final Date _start;

private final Date _end;

}

我把DateRange设为不可变，也就是说，其中所有字段都是final，只能由构造函数来赋值，因此没有任何函数可以修改其中任何字段值。这是一个明智的决定，因为这样可以避免别名带来的困扰。Java的函数参数都是按值传递的，不可变类正好能够模仿Java参数的工作方式，因此这种做法对于本项重构是最合适的。

接下来我把DateRange对象加到getFlowBetween()函数的参数列中：

class Account...

double getFlowBetween(Date start,Date end,DateRange range){

double result = 0;

Enumeration e = _entries.elements();

while (e.hasMoreElements()){

Entry each = (Entry)e.nextElement();

if (each.getDate().equals(start)||

each.getDate().equals(end)||

(each.getDate().after(start)&& each.getDate().before(end)))

{

result += each.getValue();

}

}

return result;

}

client code...

double flow = anAccount.getFlowBetween(startDate,endDate,null);

至此，只需编译一下就行了，因为我尚未修改程序的任何行为。

下一个步骤是去除旧参数之一，以新建对象取而代之。首先我删除start参数，并修改getFlowBetween()函数及其调用者，让它们转而使用新对象：

class Account...

double getFlowBetween(Date end,DateRange range){

double result = 0;

Enumeration e = _entries.elements();

while (e.hasMoreElements()){

Entry each = (Entry)e.nextElement();

if (each.getDate().equals(range.getStart())||

each.getDate().equals(end)||

(each.getDate().after(range.getStart())&& each.getDate()

.before(end)))

{

result += each.getValue();

}

}

return result;

}

client code...

double flow = anAccount.getFlowBetween(endDate,new DateRange

(startDate,null));

然后我将end参数也移除：

class Account...

double getFlowBetween(DateRange range){

double result = 0;

Enumeration e = _entries.elements();

while (e.hasMoreElements()){

Entry each = (Entry)e.nextElement();

if (each.getDate().equals(range.getStart())||

each.getDate().equals(range.getEnd())||

(each.getDate().after(range.getStart())&& each.getDate()

.before(range.getEnd())))

{

result += each.getValue();

}

}

return result;

}

client code...

double flow=anAccount.getFlowBetween(new DateRange(startDate,endDate));

现在，我已经引入了参数对象。我还可以将适当的行为从其他函数移到这个新建对象中，进一步从本项重构获得更大利益。这里，我选定条件表达式中的代码，实施Extract Method (110)和Move Method (142)，最后得到如下代码：

class Account...

double getFlowBetween(DateRange range){

double result = 0;

Enumeration e = _entries.elements();

while (e.hasMoreElements()){

Entry each = (Entry)e.nextElement();

if (range.includes(each.getDate())){

result += each.getValue();

}

}

return result;

}

class DateRange...

boolean includes(Date arg){

return (arg.equals(_start)||

arg.equals(_end)||

(arg.after(_start)&& arg.before(_end)));

}

如此单纯的提炼和搬移动作，我通常一步完成。如果在这个过程中出错，我可以回到重构前的状态，然后分成两个较小步骤重新进行。

---

## 10.10 Remove Setting Method（移除设值函数）

类中的某个字段应该在对象创建时被设值，然后就不再改变。

去掉该字段的所有设值函数。

动机

如果你为某个字段提供了设值函数，这就暗示这个字段值可以被改变。如果你不希望在对象创建之后此字段还有机会被改变，那就不要为它提供设值函数（同时将该字段设为final）。这样你的意图会更加清晰，并且可以排除其值被修改的可能性——这种可能性往往是非常大的。

如果你保留了间接访问变量的方法，就可能经常有程序员盲目使用它们[Beck]。这些人甚至会在构造函数中使用设值函数！我猜想他们或许是为了代码的一致性，但却忽视了设值函数往后可能带来的混淆。

做法

检查设值函数被使用的情况，看它是否只被构造函数调用,或者被构造函数所调用的另一个函数调用。

修改构造函数，使其直接访问设值函数所针对的那个变量。

如果某个子类通过设值函数给超类的某个private字段设了值，那么你就不能这样修改。这种情况下你应该试着在超类中提供一个protected函数（最好是构造函数）来给这些字段设值。不论你怎么做，都不要给超类中的函数起一个与设值函数混淆的名字。

编译，测试。

移除这个设值函数，将它所针对的字段设为final。
[1]

编译，测试。

范例

下面是一个简单例子：

class Account {

private String _id;

Account(String id){

setId(id);

}

void setId(String arg){

_id = arg;

}

以上代码可修改为：

class Account {

private final String _id;

Account(String id){

_id = id;

}

问题可能以几种不同的形式出现。首先，你可能会在设值函数中对传入参数做运算：

class Account {

private String _id;

Account(String id){

setId(id);

}

void setId(String arg){

_id = "ZZ" + arg;

}

如果对参数的运算很简单（就像上面这样）而且又只有一个构造函数，我可以直接在构造函数中做相同的修改。如果修改很复杂，或者有一个以上的函数调用它，就需要提供一个独立函数。我需要为新函数起个好名字，清楚表达该函数的用途：
[2]

class Account {

private final String _id; //译者注：这里的final修饰符必须去掉

Account(String id){

initializeId(id);

}

private void initializeId(String arg){

_id = "ZZ" + arg;

}

如果子类需要对超类的private变量赋初值，情况就比较麻烦一些：

class InterestAccount extends Account...

private double _interestRate;

InterestAccount(String id,double rate){

setId(id);

_interestRate = rate;

}

问题是我无法在InterestAccount()中直接访问id变量。最好的解决办法就是使用超类构造函数：

class InterestAccount...

InterestAccount(String id,double rate){

super(id);

_interestRate = rate;

}

如果不能那样做，那么使用一个命名良好的函数就是最好的选择：

class InterestAccount...

InterestAccount(String id,double rate){

initializeId(id);

_interestRate = rate;

}

另一种需要考虑的情况就是对一个集合设值：

class Person {

Vector getCourses(){

return _courses;

}

void setCourses(Vector arg){

_courses = arg;

}

private Vector _courses;

在这里，我希望将设值函数替换为add操作和remove操作。我已经在Encapsulate Collection (208)中谈到了这一点。

---

## 10.11 Hide Method（隐藏函数）

有一个函数，从来没有被其他任何类用到。

将这个函数修改为private。

动机

重构往往促使你修改函数的可见度。提高函数可见度的情况很容易想象：另一个类需要用到某个函数，因此你必须提高该函数的可见度。但是要指出一个函数的可见度是否过高，就稍微困难一些。理想状况下，你可以使用工具检查所有函数，指出可被隐藏起来的函数。即使没有这样的工具，你也应该时常进行这样的检查。

一种特别常见的情况是：当你面对一个过于丰富、提供了过多行为的接口时，就值得将非必要的取值函数和设值函数隐藏起来。尤其当你面对的是一个只有简单封装的数据容器时，情况更是如此。随着越来越多行为被放入这个类，你会发现许多取值/设值函数不再需要公开，因此可以把它们隐藏起来。如果你把取值/设值函数设为private，然后在所有地方都直接访问变量，那就可以放心移除取值/设值函数了。

做法

经常检查有没有可能降低某个函数的可见度。

使用lint一类的工具，尽可能频繁地检查。当你在另一个类中移除对某个函数的调用时，也应该进行检查。

特别对设值函数进行上述的检查。

尽可能降低所有函数的可见度。

每完成一组函数的隐藏之后，编译并测试。

如果有不适当的隐藏，编译器很自然会检验出来，因此不必每次修改后都进行编译。如有任何错误出现，很容易被发现。

---

## 10.12 Replace Constructor with Factory Method （以工厂函数取代构造函数）

你希望在创建对象时不仅仅是做简单的建构动作。

将构造函数替换为工厂函数。

Employee (int type){
_type = type;
}

static Employee create(int type){
return new Employee(type);
}

动机

使用Replace Constructor with Factory Method (304)的最显而易见的动机，就是在派生子类的过程中以工厂函数取代类型码。你可能常常需要根据类型码创建相应的对象，现在，创建名单中还得加上子类，那些子类也是根据类型码来创建。然而由于构造函数只能返回单一类型的对象，因此你需要将构造函数替换为工厂函数[Gang of Four]。

此外，如果构造函数的功能不能满足你的需要，也可以使用工厂函数来代替它。工厂函数也是Change Value to Reference (179)的基础。你也可以令你的工厂函数根据参数的个数和类型，选择不同的创建行为。

做法

新建一个工厂函数，让它调用现有的构造函数。

将调用构造函数的代码改为调用工厂函数。

每次替换后，编译并测试。

将构造函数声明为private。

编译。

范例：根据整数（实际是类型码）创建对象

又是那个单调乏味的例子：员工薪资系统。我以Employee表示“员工”：

class Employee {

private int _type;

static final int ENGINEER = 0;

static final int SALESMAN = 1;

static final int MANAGER = 2;

Employee(int type){

_type = type;

}

我希望为Employee提供不同的子类，并分别给予它们相应的类型码。因此，我需要建立一个工厂函数：

static Employee create(int type){

return new Employee(type);

}

然后，我要修改构造函数的所有调用点，让它们改用上述新建的工厂函数，并将构造函数声明为private：

client code...

Employee eng = Employee.create(Employee.ENGINEER);

class Employee...

private Employee (int type){

_type = type;

}

范例：根据字符串创建子类对象

迄今为止，我还没有获得什么实质收获。目前的好处在于：我把“对象创建请求的接收者”和“被创建对象所属的类”分开了。如果我随后使用Replace Type Code with Subclasses (223)把类型码转换为Employee的子类，就可以运用工厂函数，将这些子类对用户隐藏起来：

static Employee create(int type){

switch (type){

case ENGINEER:

return new Engineer();

case SALESMAN:

return new Salesman();

case MANAGER:

return new Manager();

default:

throw new IllegalArgumentException("Incorrect type code value");

}

}

可惜的是，这里面有一个switch语句。如果我添加一个新的子类，就必须记得更新这里的switch语句，而我又偏偏很健忘。

绕过这个switch语句的一个好办法是使用Class.forName()。第一件要做的事是修改参数类型，这从根本上说是Rename Method (273)的一种变体。首先我得建立一个函数，让它接收一个字符串参数：

static Employee create(String name){

try {

return (Employee)Class.forName(name).newInstance();

} catch (Exception e){

throw new IllegalArgumentException("Unable to instantiate" + name);

}

}

然后让稍早那个“create()函数int版”调用新建的“create()函数String版”：

class Employee {

static Employee create(int type){

switch (type){

case ENGINEER:

return create("Engineer");

case SALESMAN:

return create("Salesman");

case MANAGER:

return create("Manager");

default:

throw new IllegalArgumentException("Incorrect type code value");

}

}

然后，我得修改create()函数的调用者，将下列这样的语句：

Employee.create(ENGINEER)

修改为：

Employee.create("Engineer")

完成之后，我就可以将“create()函数int版”移除了。

现在，当我需要添加新的Employee子类时，就不再需要更新create()函数了。但我却因此失去了编译期检验，使得一个小小的拼写错误就可能造成运行期错误。如果有必要防止运行期错误，我会使用明确函数来创建对象（见本页下节）。但这样一来，每添加一个新的子类，我就必须添加一个新函数。这就是为了类型安全而牺牲掉的灵活性。还好，即使我做了错误选择，也可以使用Parameterize Method (283)或Replace Parameter with Explicit Methods (285)撤销决定。

另一个必须谨慎使用Class.forName()的原因是：它向用户暴露了子类名称。不过这并不太糟糕，因为你可以使用其他字符串，并在工厂函数中执行其他行为。这也是不使用Inline Method (117)去除工厂函数的一个好理由。

范例：以明确函数创建子类

我可以通过另一条途径来隐藏子类——使用明确函数。如果你只有少数几个子类，而且它们都不再变化，这条途径是很有用的。我可能有个抽象的Person类，它有两个子类：Male和Female。首先我在超类中为每个子类定义一个工厂函数：

class Person...

static Person createMale(){

return new Male();

}

static Person createFemale(){

return new Female();

}

然后我可以把下面的调用：

Person kent = new Male();

替换成：

Person kent = Person.createMale();

但是这就使得超类必须知晓子类。如果想避免这种情况，你需要一个更为复杂的设计，例如Product Trader模式[Bäumer and Riehle]。绝大多数情况下你并不需要如此复杂的设计，上面介绍的做法已经绰绰有余。

---

## 10.13 Encapsulate Downcast（封装向下转型）

某个函数返回的对象，需要由函数调用者执行向下转型（downcast）。

将向下转型动作移到函数中。

Object lastReading(){
return readings.lastElement();
}

Reading lastReading(){
return (Reading)readings.lastElement();
}

动机

在强类型OO语言中，向下转型是最烦人的事情之一。之所以很烦人，是因为从感觉上来说它完全没有必要：你竟然越俎代庖地告诉编译器某些应该由编译器自己计算出来的东西。但是，由于计算对象类型往往比较麻烦，你还是常常需要亲自告诉编译器对象的确切类型。向下转型在Java特别盛行，因为Java没有模板机制，因此如果你想从集合之中取出一个对象，就必须进行向下转型。
[3]

向下转型也许是一种无法避免的罪恶，但你仍然应该尽可能少做。如果你的某个函数返回一个值，并且你知道所返回的对象类型比函数签名所昭告的更特化，你便是在函数用户身上强加了非必要的工作。这种情况下你不应该要求用户承担向下转型的责任，应该尽量为他们提供准确的类型。

以上所说的情况，常会在返回迭代器或集合的函数身上发生。此时你就应该观察人们拿这个迭代器干什么用，然后有针对性地提供专用函数。

做法

找出必须对函数调用结果进行向下转型的地方。

这种情况通常出现在返回一个集合或迭代器的函数中。

将向下转型动作搬移到该函数中。

针对返回集合的函数，使用Encapsulate Collection (208)。

范例

下面的例子中，我以Reading表示“书籍”。我还拥有一个名为lastReading()的函数，它从一个用于保存Reading对象的vector中返回其最后一个元素：

Object lastReading(){

return readings.lastElement();

}

我应该将这个函数变成：

Reading lastReading(){

return (Reading)readings.lastElement();

}

当我拥有一个集合时，上述那么做就很有意义。如果“保存Reading对象”的集合被放在Site类中，并且我看到了如下的客户端代码：

Reading lastReading = (Reading)theSite.readings().lastElement()

我就可以不再把向下转型的工作推给用户，并得以向用户隐藏集合：

Reading lastReading = theSite.lastReading();

class Site...

Reading lastReading(){

return (Reading)readings().lastElement();

}

如果你修改函数，将其返回类型改为原返回类型的子类，那就是改变了函数签名，但并不会破坏客户端代码，因为编译器知道它总是可以将一个子类自动向上转型为超类。当然你必须确保这个子类不会破坏超类带来的任何契约。

---

## 10.14 Replace Error Code with Exception （以异常取代错误码）

某个函数返回一个特定的代码，用以表示某种错误情况。

改用异常。

int withdraw(int amount){

if (amount > _balance)

return -1;

else {

_balance -= amount;

return 0;

}

}

void withdraw(int amount)throws BalanceException {
if (amount > _balance)throw new BalanceException();
_balance -= amount;
}

动机

和生活一样，计算机偶尔也会出错。一旦事情出错，你就需要有些对策。最简单的情况下，你可以停止程序运行，返回一个错误码。这就好像因为错过一班飞机而自杀一样（如果真那么做，哪怕我是只猫，我的九条命也早赔光了）。尽管我的油腔滑调企图带来一点幽默，但这种“软件自杀”选择的确是有好处的。如果程序崩溃代价很小，用户又足够宽容，那么就放心终止程序的运行好了。但如果你的程序比较重要，就需要以更认真的方式来处理。

问题在于：程序中发现错误的地方，并不一定知道如何处理错误。当一段子程序发现错误时，它需要让它的调用者知道这个错误，而调用者也可能将这个错误继续沿着调用链传递上去。许多程序都使用特殊输出来表示错误，Unix系统和C-based系统的传统方式就是以返回值表示子程序的成功或失败。

Java有一种更好的错误处理方式：异常。这种方式之所以更好，因为它清楚地将“普通程序”和“错误处理”分开了，这使得程序更容易理解——我希望你如今已经坚信：代码的可理解性应该是我们虔诚追求的目标。

做法

决定应该抛出受控（checked）异常还是非受控（unchecked）异常。

如果调用者有责任在调用前检查必要状态，就抛出非受控异常。

如果想抛出受控异常，你可以新建一个异常类，也可以使用现有的异常类。

找到该函数的所有调用者，对它们进行相应调整，让它们使用异常。

如果函数抛出非受控异常，那么就调整调用者，使其在调用函数前做适当检查。每次修改后，编译并测试。

如果函数抛出受控异常，那么就调整调用者，使其在try区段中调用该函数。

修改该函数的签名，令它反映出新用法。

如果函数有许多调用者，上述修改过程可能跨度太大。你可以将它分成下列数个步骤。

决定应该抛出受控异常还是非受控异常。

新建一个函数，使用异常来表示错误状况，将旧函数的代码复制到新函数中，并做适当调整。

修改旧函数的函数本体，让它调用上述新建函数。

编译，测试。

逐一修改旧函数的调用者，令其调用新函数。每次修改后，编译并测试。

移除旧函数。

范例

现实生活中你可以透支你的账户余额，计算机教科书却总是假设你不能这样做，这不是很奇怪吗？不过下面的例子仍然假设你不能这样做：

class Account...

int withdraw(int amount){

if (amount > _balance)

return -1;

else {

_balance -= amount;

return 0;

}

}

private int _balance;

为了让这段代码使用异常，我首先需要决定使用受控异常还是非受控异常。决策关键在于：调用者是否有责任在取款之前检查存款余额，还是应该由withdraw()函数负责检查。如果“检查余额”是调用者的责任，那么“取款金额大于存款余额”就是一个编程错误。由于这是一个编程错误，所以我应该使用非受控异常。另一方面，如果“检查余额”是withdraw()函数的责任，我就必须在函数接口中声明它可能抛出这个异常，那么也就提醒了调用者注意这个异常，并采取相应措施。

范例：非受控异常

首先考虑非受控异常。使用这个东西就表示应该由调用者负责检查。首先我需要检查调用端的代码，它不应该使用withdraw()函数的返回值，因为该返回值只用来指出程序员的错误。如果我看到下面这样的代码：

if (account.withdraw(amount)== -1)

handleOverdrawn();

else doTheUsualThing();

我应该将它替换为这样的代码：

if (!account.canWithdraw(amount))

handleOverdrawn();

else {

account.withdraw(amount);

doTheUsualThing();

}

每次修改后，编译并测试。

现在，我需要移除错误码，并在程序出错时抛出异常。由于这种行为是异常的、罕见的，所以我应该用一个卫语句检查这种情况：

void withdraw(int amount){

if (amount > _balance)

throw new IllegalArgumentException ("Amount too large");

_balance -= amount;

}

由于这是程序员所犯的错误，所以我应该使用断言更清楚地指出这一点：

class Account...

void withdraw(int amount){

Assert.isTrue("sufficient funds",amount <= _balance);

_balance -= amount;

}

class Assert...

static void isTrue(String comment,boolean test){

if (!test){

throw new RuntimeException("Assertion failed: " + comment);

}

}

范例：受控异常

受控异常的处理方式略有不同。首先我要建立（或使用）一个合适的异常：

class BalanceException extends Exception {}

然后，调整调用端如下：

try {

account.withdraw(amount);

doTheUsualThing();

} catch (BalanceException e){

handleOverdrawn();

}

接下来我要修改withdraw()函数，让它以异常表示错误状况：

void withdraw(int amount)throws BalanceException {

if (amount > _balance)throw new BalanceException();

_balance -= amount;

}

这个过程的麻烦在于：我必须一次性修改所有调用者和被它们调用的函数，否则编译器会报错。如果调用者很多，这个步骤就实在太大了，其中没有编译和测试的保障。

这种情况下，我可以借助一个临时中间函数。我仍然从先前相同的情况出发：

if (account.withdraw(amount)== -1)

handleOverdrawn();

else doTheUsualThing();

class Account ...

int withdraw(int amount){

if (amount > _balance)

return -1;

else {

_balance -= amount;

return 0;

}

}

首先，创建一个newWithdraw()函数，让它抛出异常：

void newWithdraw(int amount)throws BalanceException {

if (amount > _balance)throw new BalanceException();

_balance -= amount;

}

然后，调整现有的withdraw()函数，让它调用newWithdraw()：

int withdraw(int amount){

try {

newWithdraw(amount);

return 0;

} catch (BalanceException e){

return -1;

}

}

完成以后，编译并测试。现在我可以逐一将调用旧函数的地方改为调用新函数：

try {

account.newWithdraw(amount);

doTheUsualThing();

} catch (BalanceException e){

handleOverdrawn();

}

由于新旧两个函数都存在，所以每次修改后我都可以编译、测试。所有调用者都修改完毕后，旧函数便可移除，并使用Rename Method (273)修改新函数名称，使它与旧函数相同。

---

## 10.15 Replace Exception with Test（以测试取代异常）

面对一个调用者可以预先检查的条件，你抛出了一个异常。

修改调用者，使它在调用函数之前先做检查。

double getValueForPeriod(int periodNumber){
try {
return _values[periodNumber];
} catch (ArrayIndexOutOfBoundsException e){
return 0;
}
}

double getValueForPeriod(int periodNumber){
if (periodNumber >= _values.length)return 0;
return _values[periodNumber];
}

动机

异常的出现是程序语言的一大进步。运用Replace Error Code with Exception (310)，异常便可协助我们避免很多复杂的错误处理逻辑。但是，就像许多好东西一样，异常也会被滥用，从而变得不再让人愉快（就连味道极好的Aventinus啤酒，喝得太多也会让我厌烦[Jackson]）。“异常”只应该被用于异常的、罕见的行为，也就是那些产生意料之外的错误的行为，而不应该成为条件检查的替代品。如果你可以合理期望调用者在调用函数之前先检查某个条件，那么就应该提供一个测试，而调用者应该使用它。

做法

在函数调用点之前，放置一个测试语句，将函数内catch区段中的代码复制到测试句的适当if分支中。

在catch区段起始处加入一个断言，确保catch区段绝对不会被执行。

编译，测试。

移除所有catch区段，然后将try区段内的代码复制到try之外，然后移除try区段。

编译，测试。

范例

下面的例子中，我以一个ResourcePool对象管理一些创建代价高昂而又可以重复使用的资源（例如数据库连接）。这个对象带有两个“池”（pool）：一个用以保存可用资源，一个用以保存已分配资源。当用户请求一份资源时，ResourcePool对象从“可用资源池”中取出一份资源交出，并将这份资源转移到“已分配资源池”。当用户释放一份资源时，ResourcePool对象就将该资源从“已分配资源池”放回“可用资源池”。如果“可用资源池”不能满足用户的请求，ResourcePool对象就创建一份新资源。

资源供应函数可能如下所示：

class ResourcePool

Resource getResource(){

Resource result;

try {

result = (Resource)_available.pop();

_allocated.push(result);

return result;

} catch (EmptyStackException e){

result = new Resource();

_allocated.push(result);

return result;

}

}

Stack _available;

Stack _allocated;

在这里，“可用资源用尽”并不是一种意料外的事件，因此我不该使用异常表示这种情况。

为了去掉这里的异常，我首先必须添加一个适当的提前测试，并在其中处理“可用资源池为空”的情况：

Resource getResource(){

Resource result;

if (_available.isEmpty()){

result = new Resource();

_allocated.push(result);

return result;

}

else {

try {

result = (Resource)_available.pop();

_allocated.push(result);

return result;

} catch (EmptyStackException e){

result = new Resource();

_allocated.push(result);

return result;

}

}

}

现在getResource()应该绝对不会抛出异常了。我可以添加断言保证这一点：

Resource getResource(){

Resource result;

if (_available.isEmpty()){

result = new Resource();

_allocated.push(result);

return result;

} else {

try {

result = (Resource)_available.pop();

_allocated.push(result);

return result;

} catch (EmptyStackException e){

Assert.shouldNeverReachHere("available was empty on pop");

result = new Resource();

_allocated.push(result);

return result;

}

}

}

class Assert...

static void shouldNeverReachHere(String message){

throw new RuntimeException(message);

}

编译并测试。如果一切运转正常，就可以将try区段中的代码复制到try区段之外，然后将try区段全部移除：

Resource getResource(){

Resource result;

if (_available.isEmpty()){

result = new Resource();

_allocated.push(result);

return result;

}

else {

result = (Resource)_available.pop();

_allocated.push(result);

return result;

}

}

在这之后我常常发现，可以对条件代码加以整理。本例之中我可以使用Consolidate Duplicate Conditional Fragments (243)：

Resource getResource(){

Resource result;

if (_available.isEmpty())

result = new Resource();

else

result = (Resource)_available.pop();

_allocated.push(result);

return result;

}

[1].
 本步骤必须在本重构的最后进行，详情请看 http://www.refactoring.com/catalog/removeSetting Method.html和稍后的译者注。——译者注

[2].
 此时不能将独立函数中要赋值的字段──即此处的_id字段──声明为final，否则不能通过编译。因此这一段所描述的重构手法实际上并不成立：Account在重构后仍然是可变对象。唯一能够得到的好处是：通过修改设值函数的名称，可以让读者明白initializeId函数只应该用于对象构造阶段。——译者注

[3].
 自从Java 5加入模板机制以后，非向下转型不可的场合几乎绝迹。读者如果发现自己写出需要向下转型的代码，在考虑使用本重构手法之前，应该首先考虑是否可以代之以模板类。——译者注

---


# 第11章 处理概括关系 (Dealing with Generalization)

## 11.1 Pull Up Field（字段上移）

两个子类拥有相同的字段。

将该字段移至超类。

动机

如果各子类是分别开发的，或者是在重构过程中组合起来的，你常会发现它们拥有重复特性，特别是字段更容易重复。这样的字段有时拥有近似的名字，但也并非绝对如此。判断若干字段是否重复，唯一的办法就是观察函数如何使用它们。如果它们被使用的方式很相似，你就可以将它们归纳到超类去。

本项重构从两方面减少重复：首先它去除了重复的数据声明；其次它使你可以将使用该字段的行为从子类移至超类，从而去除重复的行为。

做法

针对待提升之字段，检查它们的所有被使用点，确认它们以同样的方式被使用。

如果这些字段的名称不同，先将它们改名，使每一个名称都和你想为超类字段取的名称相同。

编译，测试。

在超类中新建一个字段。

如果这些字段是private的，你必须将超类的字段声明为protected，这样子类才能引用它。

移除子类中的字段。

编译，测试。

考虑对超类的新建字段使用Self Encapsulate Field (171)。

---

## 11.2 Pull Up Method（函数上移）

有些函数，在各个子类中产生完全相同的结果。

将该函数移至超类。

动机

避免行为重复是很重要的。尽管重复的两个函数也可以各自工作得很好，但重复自身只会成为错误的滋生地，此外别无价值。无论何时，只要系统之内出现重复，你就会面临“修改其中一个却未能修改另一个”的风险。通常，找出重复也有一定困难。

如果某个函数在各子类中的函数体都相同（它们很可能是通过复制粘贴得到的），这就是最显而易见的Pull Up Method (322)适用场合。当然，情况并不总是如此明显。你也可以只管放心地重构，再看看测试程序会不会发牢骚，但这就需要对你的测试有充分的信心。我发现，观察这些可能重复的函数之间的差异往往大有收获：它们经常会向我展示那些我忘记测试的行为。

Pull Up Method (322)常常紧随其他重构而被使用。也许你能找出若干个身处不同子类内的函数，而它们又可以通过某种形式的参数调整成为相同的函数。这时候，最简单的办法就是首先分别调整这些函数的参数，然后再将它们概括到超类中。当然，如果你足够自信，也可以一次完成这两个步骤。

有一种特殊情况也需要使用Pull Up Method (322)：子类的函数覆写了超类的函数，但却仍然做相同的工作。

Pull Up Method (322)过程中最麻烦的一点就是：被提升的函数可能会引用只出现于子类而不出现于超类的特性。如果被引用的是个函数，你可以将该函数也一同提升到超类，或者在超类中建立一个抽象函数。在此过程中，你可能需要修改某个函数的签名，或建立一个委托函数。

如果两个函数相似但不相同，你或许可以先借助Form Template Method (345)构造出相同的函数，然后再提升它们。

做法

检查待提升函数，确定它们是完全一致的。

如果这些函数看上去做了相同的事，但并不完全一致，可使用Substitute Algorithm (139)让它们变得完全一致。

如果待提升函数的签名不同，将那些签名都修改为你想要在超类中使用的签名。

在超类中新建一个函数，将某一个待提升函数的代码复制到其中，做适当调整，然后编译。

如果你使用的是一种强类型语言，而待提升函数又调用了一个只出现于子类而未出现于超类的函数，你可以在超类中为被调用函数声明一个抽象函数。

如果待提升函数使用了子类的一个字段，你可以使用Pull Up Field (320)将该字段也提升到超类；或者也可以先使用Self Encapsulate Field (171)，然后在超类中把取值函数声明为抽象函数。

移除一个待提升的子类函数。

编译，测试。

逐一移除待提升的子类函数，直到只剩下超类中的函数为止。每次移除之后都需要测试。

观察该函数的调用者，看看是否可以改为使用超类类型的对象。

范例

我以Customer表示“顾客”，它有两个子类：表示“普通顾客”的Regular-Customer和表示“贵宾”的PreferredCustomer。

两个子类都有一个createBill()函数，并且代码完全一样：

void createBill (date Date){

double chargeAmount = chargeFor(lastBillDate,date);

addBill (date,charge);

}

但我不能直接把这个函数上移到超类，因为各个子类的chargeFor()函数并不相同。我必须先在超类中声明chargeFor()抽象函数：

class Customer...

abstract double chargeFor(date start,date end)

然后，我就可以将createBill()函数从其中一个子类复制到超类。复制完之后应该编译，然后移除那个子类的createBill()函数，再编译并测试。随后再移除另一个子类的createBill()函数，再次编译并测试：

---

## 11.3 Pull Up Constructor Body（构造函数本体上移）

你在各个子类中拥有一些构造函数，它们的本体几乎完全一致。

在超类中新建一个构造函数，并在子类构造函数中调用它。

class Manager extends Employee...
public Manager(String name,String id,int grade){
_name = name;
_id = id;
_grade = grade;
}

public Manager(String name,String id,int grade){
super(name,id);
_grade = grade;
}

动机

构造函数是很奇妙的东西。它们不是普通函数，使用它们比使用普通函数受到更多的限制。

如果你看见各个子类中的函数有共同行为，第一个念头应该是将共同行为提炼到一个独立函数中，然后将这个函数提升到超类。对构造函数而言，它们彼此的共同行为往往就是“对象的建构”。这时候你需要在超类中提供一个构造函数，然后让子类都来调用它。很多时候，子类构造函数的唯一动作就是调用超类构造函数。这里不能运用Pull Up Method (322），因为你无法在子类中继承超类构造函数。（你可曾痛恨过这个规定？）

如果重构过程过于复杂，你可以考虑转而使用Replace Constructor with Factory Method (304)。

做法

在超类中定义一个构造函数。

将子类构造函数中的共同代码搬移到超类构造函数中。

被搬移的可能是子类构造函数的全部内容。

首先设法将共同代码搬移到子类构造函数起始处，然后再复制到超类构造函数中。

将子类构造函数中的共同代码删掉，改而调用新建的超类构造函数。

如果子类构造函数中的所有代码都是一样的，那么子类构造函数就只需要调用超类构造函数。

编译，测试。

如果日后子类构造函数再出现共同代码，你可以首先使用Extract Method (110）将那一部分提炼到一个独立函数，然后使用Pull Up Method (322）将该函数上移到超类。

范例

下面是一个表示“雇员”的Employee类和一个表示“经理”的Manager类：

class Employee...

protected String _name;

protected String _id;

class Manager extends Employee...

public Manager(String name,String id,int grade){

_name = name;

_id = id;

_grade = grade;

}

private int _grade;

Employee的字段应该在Employee构造函数中设初值。因此我定义了一个Employee构造函数，并将它声明为protected，表示子类应该调用它：

class Employee

protected Employee (String name,String id){

_name = name;

_id = id;

}

然后，我从子类中调用它：

public Manager (String name,String id,int grade){

super (name,id);

_grade = grade;

}

后来情况又有些变化，构造函数中出现了共同代码。假如我有以下代码：

class Employee...

boolean isPriviliged(){..}

void assignCar(){..}

class Manager...

public Manager(String name,String id,int grade){

super(name,id);

_grade = grade;

if (isPriviliged())assignCar(); // every subclass does this

}

boolean isPriviliged(){

return _grade > 4;

}

我不能把调用assignCar()的行为移到超类构造函数中，因为唯有把合适的值赋给_grade字段后才能执行assignCar()。此时我需要使用Extract Method (110)和Pull up Method (322)。

class Employee...

void initialize(){

if (isPriviliged())assignCar();

}

class Manager...

public Manager(String name,String id,int grade){

super(name,id);

_grade = grade;

initialize();

}

---

## 11.4 Push Down Method（函数下移）

超类中的某个函数只与部分（而非全部）子类有关。

将这个函数移到相关的那些子类去。

动机

Push Down Method (328)与Pull Up Method (322)恰恰相反。当我有必要把某些行为从超类移至特定的子类时，我就使用Push Down Method (328)，它通常也只在这种时候有用。使用Extract Subclass (330)之后你可能会需要它。

做法

在所有子类中声明该函数，将超类中的函数本体复制到每一个子类函数中。

你可能需要将超类的某些字段声明为protected，让子类函数也能够访问它们。如果日后你也想把这些字段下移到子类，通常就可以那么做；否则应该使用超类提供的访问函数。如果访问函数并非public，你得将它声明为protected。

删除超类中的函数。

你可能必须修改调用端的某些变量声明或参数声明，以便能够使用子类。

如果有必要通过一个超类对象访问该函数，或你不想把该函数从任何子类中移除，再或超类是抽象类，那么你就可以在超类中把该函数声明为抽象函数。

编译，测试。

将该函数从所有不需要它的那些子类中删掉。

编译，测试。

---

## 11.5 Push Down Field（字段下移）

超类中的某个字段只被部分（而非全部）子类用到。

将这个字段移到需要它的那些子类去。

动机

Push Down Field (329)与Pull Up Field (320)恰恰相反：如果只有某些（而非全部）子类需要超类内的一个字段，你可以使用本项重构。

做法

在所有子类中声明该字段。

将该字段从超类中移除。

编译，测试。

将该字段从所有不需要它的那些子类中删掉。

编译，测试。

---

## 11.6 Extract Subclass（提炼子类）

类中的某些特性只被某些（而非全部）实例用到。

新建一个子类，将上面所说的那一部分特性移到子类中。

动机

使用Extract Subclass (330)的主要动机是：你发现类中的某些行为只被一部分实例用到，其他实例不需要它们。有时候这种行为上的差异是通过类型码区分的，此时你可以使用Replace Type Code with Subclasses (223)或Replace Type Code with State/Strategy (227)。但是，并非一定要出现了类型码才表示需要考虑使用子类。

Extract Class (149)是Extract Subclass (330)之外的另一种选择，两者之间的抉择其实就是委托和继承之间的抉择。Extract Subclass (330)通常更容易进行，但它也有限制：一旦对象创建完成，你无法再改变与类型相关的行为。但如果使用Extract Class (149)，你只需插入另一个组件就可以改变对象的行为。此外，子类只能用以表现一组变化。如果你希望一个类以几种不同的方式变化，就必须使用委托。

做法

为源类定义一个新的子类。

为这个新的子类提供构造函数。

简单的做法是：让子类构造函数接受与超类构造函数相同的参数，并通过super调用超类构造函数。

如果你希望对用户隐藏子类的存在，可使用Replace Constructor with Factory Method (304)。

找出调用超类构造函数的所有地点。如果它们需要的是新建的子类，令它们改而调用新构造函数。

如果子类构造函数需要的参数和超类构造函数的参数不同，可以使用Rename Method (273)修改其参数列。如果子类构造函数不需要超类构造函数的某些参数，可以使用Rename Method (273)将它们去除。

如果不再需要直接创建超类的实例，就将超类声明为抽象类。

逐一使用Push Down Method (328)和Push Down Field (329)将源类的特性移到子类去。

和Extract Class (149)不同的是，先处理函数再处理数据，通常会简单一些。当一个public函数被下移到子类后，你可能需要重新定义该函数的调用端的局部变量或参数类型，让它们改而调用子类中的新函数。如果忘记进行这一步骤，编译器会提醒你。

找到所有这样的字段：它们所传达的信息如今可由继承体系自身传达（这一类字段通常是boolean变量或类型码）。以Self Encapsulate Field (171)避免直接使用这些字段，然后将它们的取值函数替换为多态常量函数。所有使用这些字段的地方都应该以Replace Conditional with Polymorphism (255)重构。

任何函数如果位于源类之外，而又使用了上述字段的访问函数，考虑以Move Method (142)将它移到源类中，然后再使用Replace Conditional with Polymorphism (255)。

每次下移之后，编译并测试。

范例

下面是JobItem类，用来决定当地修车厂的工作报价：

class JobItem ...

public JobItem(int unitPrice,int quantity,boolean isLabor,Employee

employee){

_unitPrice = unitPrice;

_quantity = quantity;

_isLabor = isLabor;

_employee = employee;

}

public int getTotalPrice(){

return getUnitPrice()* _quantity;

}

public int getUnitPrice(){

return (_isLabor)?

_employee.getRate():

_unitPrice;

}

public int getQuantity(){

return _quantity;

}

public Employee getEmployee(){

return _employee;

}

private int _unitPrice;

private int _quantity;

private Employee _employee;

private boolean _isLabor;

class Employee...

public Employee(int rate){

_rate = rate;

}

public int getRate(){

return _rate;

}

private int _rate;

我要提炼出一个LaborItem子类，因为上述某些行为和数据只在按工时（labor）收费的情况下才需要。首先建立这样一个类：

class LaborItem extends JobItem {}

我需要为LaborItem提供一个构造函数，因为JobItem没有默认构造函数。我把超类构造函数的参数列复制过来：

public LaborItem (int unitPrice,int quantity,boolean isLabor,Employee

employee){

super (unitPrice,quantity,isLabor,employee);

}

这就足以让新的子类通过编译了。但是这个构造函数会造成混淆：某些参数是LaborItem所需要的，另一些不是。稍后我再来解决这个问题。

下一步是要找出对JobItem构造函数的调用，并从中找出可以改用LaborItem构造函数的地方。因此，下列语句：

JobItem j1 = new JobItem (0,5,true,kent);

就被修改为：

JobItem j1 = new LaborItem (0,5,true,kent);

此时我尚未修改变量类型，只是修改了构造函数所属的类。之所以这样做，是因为我希望只在必要地点才使用新类型。到目前为止，子类还没有专属接口，因此我还不想宣布任何改变。

现在正是清理构造函数参数列的好时机。我将针对每个构造函数使用Rename Method (273)。首先处理超类构造函数。我要新建一个构造函数，并把旧构造函数声明为protected（不能直接声明为private，因为子类还需要它）：

class JobItem...

protected JobItem (int unitPrice,int quantity,boolean isLabor,Employee

employee){

_unitPrice = unitPrice;

_quantity = quantity;

_isLabor = isLabor;

_employee = employee;

}

public JobItem (int unitPrice,int quantity){

this (unitPrice,quantity,false,null)

}

现在，外部调用应该使用新构造函数：

JobItem j2 = new JobItem (10,15);

编译、测试都通过后，我再使用Rename Method (273)修改子类构造函数：

class LaborItem

public LaborItem (int quantity,Employee employee){

super (0,quantity,true,employee);

}

此时我仍然暂时使用protected的超类构造函数。

现在，我可以将JobItem的特性向下搬移。先从函数开始，我先运用Push Down Method (328)对付getEmployee()函数：

class LaborItem...

public Employee getEmployee(){

return _employee;

}

class JobItem...

protected Employee _employee;

因为_employee字段也将在稍后被下移到LaborItem，所以我现在先将它声明为protected。

将_employee字段声明为protected之后，我可以再次清理构造函数，让_employee只在即将去达的子类中被初始化：

class JobItem...

protected JobItem (int unitPrice,int quantity,boolean isLabor){

_unitPrice = unitPrice;

_quantity = quantity;

_isLabor = isLabor;

}

class LaborItem ...

public LaborItem (int quantity,Employee employee){

super (0,quantity,true);

_employee = employee;

}

_isLabor字段所传达的信息，现在已经成为继承体系的内在信息，因此我可以移除这个字段了。最好的方式是：先使用Self Encapsulate Field (171)，然后再修改访问函数，改用多态常量函数——这样的函数会在不同的子类实现版本中返回不同的固定值：

class JobItem...

protected boolean isLabor(){

return false;

}

class LaborItem...

protected boolean isLabor(){

return true;

}

然后，我就可以摆脱_isLabor字段了。

现在，我可以观察isLabor()函数的用户，并运用Replace Conditional with Polymorphism (255)重构它们。我找到了下列这样的函数：

class JobItem...

public int getUnitPrice(){

return (isLabor())?

_employee.getRate():

_unitPrice;

}

将它重构为：

class JobItem...

public int getUnitPrice(){

return _unitPrice;

}

class LaborItem...

public int getUnitPrice(){

return _employee.getRate();

}

当使用某项字段的函数全被下移至子类后，我就可以使用Push Down Field (329)将字段也下移。如果尚无法移动字段，那就表示我需要对函数做更多处理，可能需要实施Push Down Method (328)或Replace Conditional with Polymorphism (255)。

由于只有按零件收费的工作项才会用到_unitPrice字段，所以我可以再次运用Extract Subclass (330)对JobItem提炼出一个子类：PartsItem。完成后，我可以将JobItem声明为抽象类。

---

## 11.7 Extract Superclass（提炼超类）

两个类有相似特性。

为这两个类建立一个超类，将相同特性移至超类。

动机

重复代码是系统中最糟糕的东西之一。如果你在不同地方做同一件事情，一旦需要修改那些动作，你就得平白做更多的修改。

重复代码的某种形式就是：两个类以相同的方式做类似的事情，或者以不同的方式做类似的事情。对象提供了一种简化这种情况的机制，那就是继承。但是，在建立这些具有共通性的类之前，你往往无法发现这样的共通性，因此经常会在具有共通性的类出现之后，再开始建立其间的继承结构。

另一种选择就是Extract Class (149)。这两种方案之间的选择其实就是继承和委托之间的选择。如果两个类可以共享行为，也可以共享接口，那么继承是比较简单的做法。如果你选错了，也总有Replace Inheritance with Delegation (352)这瓶后悔药可吃。

做法

为原本的类新建一个空白的抽象超类。

运用Pull Up Field (320)、Pull Up Method (322)和Pull Up Constructor Body (325)逐一将子类的共同元素上移到超类。

先搬移字段，通常比较简单。

如果相应的子类函数有不同的签名，但用途相同，可以先使用Rename Method (273)将它们的签名改为相同，然后再使用Pull Up Method (322)。

如果相应的子类函数有相同的签名，但函数本体不同，可以在超类中把它们的共同签名声明为抽象函数。

如果相应的子类函数有不同的函数本体，但用途相同，可试着使用Substitute Algorithm (139)把其中一个函数的函数本体复制到另一个函数中。如果运转正常，你就可以使用Pull Up Method (322)。

每次上移后，编译并测试。

检查留在子类中的函数，看它们是否还有共通成分。如果有，可以使用Extract Method (110)将共通部分再提炼出来，然后使用Pull Up Method (322)将提炼出的函数上移到超类。如果各个子类中某个函数的整体流程很相似，你也许可以使用Form Template Method (345)。

将所有共通元素都上移到超类之后，检查子类的所有用户。如果它们只使用共同接口，你就可以把它们请求的对象类型改为超类。

范例

下面例中，我以Employee表示“员工”，以Department表示“部门”：

class Employee...

public Employee(String name,String id,int annualCost){

_name = name;

_id = id;

_annualCost = annualCost;

}

public int getAnnualCost(){

return _annualCost;

}

public String getId(){

return _id;

}

public String getName(){

return _name;

}

private String _name;

private int _annualCost;

private String _id;

public class Department...

public Department(String name){

_name = name;

}

public int getTotalAnnualCost(){

Enumeration e = getStaff();

int result = 0;

while (e.hasMoreElements()){

Employee each = (Employee)e.nextElement();

result += each.getAnnualCost();

}

return result;

}

public int getHeadCount(){

return _staff.size();

}

public Enumeration getStaff(){

return _staff.elements();

}

public void addStaff(Employee arg){

_staff.addElement(arg);

}

public String getName(){

return _name;

}

private String _name;

private Vector _staff = new Vector();

这里有两处共同点。首先，员工和部门都有名称；其次，它们都有年度成本，只不过计算方式略有不同。我要提炼出一个超类，用以包容这些共通特性。第一步是新建这个超类，并将现有的两个类定义为其子类：

abstract class Party {}

class Employee extends Party...

class Department extends Party...

然后我开始把特性上移至超类。先实施Pull up Field (320)通常会比较简单：

class Party...

protected String _name;

然后，我可以使用Pull Up Method (322)把这个字段的取值函数也上移至超类：

class Party {

public String getName(){

return _name;

}

我通常会把这个字段声明为private。不过，在此之前，我需要先使用Pull Up Constructor Body (325)，这样才能对_name正确赋值：

class Party...

protected Party(String name){

_name = name;

}

private String _name;

class Employee...

public Employee(String name,String id,int annualCost){

super(name);

_id = id;

_annualCost = annualCost;

}

class Department...

public Department(String name){

super(name);

}

Department.getTotalAnnualCost()和Employee.getAnnualCost()两个函数的用途相同，因此它们应该有相同的名称。我先运用Rename Method (273)把它们的名称改为相同：

class Department extends Party {

public int getAnnualCost(){

Enumeration e = getStaff();

int result = 0;

while (e.hasMoreElements()){

Employee each = (Employee)e.nextElement();

result += each.getAnnualCost();

}

return result;

}

它们的函数本体仍然不同，因此我目前还无法使用Pull Up Method (322)。但是我可以在超类中声明一个抽象函数：

abstract public int getAnnualCost()

这一步修改完成后，我需要观察两个子类的用户，看看是否可以改变它们转而使用新的超类。用户之一就是Department自身，它保存了一个Employee对象集合。Department.getAnnualCost()只调用集合内的元素 (对象）的getAnnualCost()函数，而该函数目前是在Party中声明的：

class Department...

public int getAnnualCost(){

Enumeration e = getStaff();

int result = 0;

while (e.hasMoreElements()){

Party each = (Party)e.nextElement();

result += each.getAnnualCost();

}

return result;

}

这一行为暗示一种新的可能性：我可以用Composite模式[Gang of Four]来对待Department和Employee，这样就可以让一个Department对象包容另一个Department对象。这是一项新功能，所以这项修改严格来说不属于重构范围。如果用户恰好需要Composite模式，我可以修改_staff字段名字，使其更好地表现这一模式。这一修改还会带来其他相应修改：修改addStaff()函数名称，并将该函数的参数类型改为Party。最后还需要把headCount()函数变成一个递归调用。我的做法是在Employee中建立一个headCount()函数，让它返回1；再使用Substitute Algorithm (139)修改Department的headCount()函数，让它加总各部门的headCount()调用结果。

---

## 11.8 Extract Interface（提炼接口）

若干客户使用类接口中的同一子集，或者两个类的接口有部分相同。

将相同的子集提炼到一个独立接口中。

动机

类之间彼此互用的方式有若干种。“使用一个类”通常意味用到该类的所有责任区。另一种情况是，某一组客户只使用类责任区中的一个特定子集。再一种情况则是，这个类需要与所有协助处理某些特定请求的类合作。

对于后两种情况，将真正用到的这部分责任分离出来通常很有意义，因为这样可以使系统的用法更清晰，同时也更容易看清系统的责任划分。如果新的类需要支持上述子集，也比较能够看清子集内有些什么东西。

在许多面向对象语言中，这种责任划分是通过多继承（multiple inheritance）来实现的。你可以针对每组行为建立一个类，再将它们组合于同一个实现中。Java只提供单继承（single inheritance），但你可以运用接口（interface）来昭示并实现上述需求。接口对于Java程序的设计方式有着巨大的影响，就连Smalltalk程序员都认为接口是一大进步!

Extract Superclass (336)和Extract Interface (341)之间有些相似之处。Extract Interface (341)只能提炼共通接口，不能提炼共通代码。使用Extract Interface (341)可能造成难闻的“重复”坏味道，幸而你可以运用Extract Class (149)先把共通行为放进一个组件中，然后将工作委托该组件，从而解决这个问题。如果有不少共通行为，Extract Superclass (336)会比较简单，但是每个类只能有一个超类。

如果某个类在不同环境下扮演截然不同的角色，使用接口就是个好主意。你可以针对每个角色以Extract Interface (341)提炼出相应接口。另一种可以用上Extract Interface (341)的情况是：你想要描述一个类的外部依赖接口（outbound interface，即这个类要求服务提供方提供的操作）。如果你打算将来加入其他种类的服务对象，只需要求它们实现这个接口即可。

做法

新建一个空接口。

在接口中声明待提炼类的共通操作。

让相关的类实现上述接口。

调整客户端的类型声明，令其使用该接口。

范例

TimeSheet类表示员工为客户工作的时间表，从中可以计算客户应该支付的费用。为了计算这笔费用，TimeSheet需要知道员工级别，以及该员工是否有特殊技能：

double charge(Employee emp,int days){

int base = emp.getRate()* days;

if (emp.hasSpecialSkill())

return base * 1.05;

else return base;

}

除了提供员工的级别和特殊技能信息外，Employee还有很多其他方面的功能，但本应用程序只需这两项功能。我可以针对这两项功能定义一个接口，从而强调“我只需要这部分功能”的事实：

interface Billable {

public int getRate();

public boolean hasSpecialSkill();

}

然后，我声明让Employee实现这个接口：

class Employee implements Billable ...

完成以后，我可以修改charge()函数声明，强调该函数只使用Employee的这部分行为：

double charge(Billable emp,int days){

int base = emp.getRate()* days;

if (emp.hasSpecialSkill())

return base * 1.05;

else return base;

}

到目前为止，我们只不过是在文档化方面有一点收获。单就这一个函数而言，这样的收获并没有太大价值；但如果有若干个类都使用Billable接口，它就会很有用。如果我还想计算电脑租金，巨大的收获就显露出来了：要想计算客户租用电脑的费用，我只需让Computer类实现Billable接口，然后就可以把租用电脑的时间也填到时间表上了。

---

## 11.9 Collapse Hierarchy（折叠继承体系）

超类和子类之间无太大区别。

将它们合为一体。

动机

如果你曾经编写过继承体系，就会知道，继承体系很容易变得过分复杂。所谓重构继承体系，往往是将函数和字段在体系中上下移动。完成这些动作后，你很可能发现某个子类并未带来该有的价值，因此需要把超类与子类合并起来。

做法

选择你想移除的类：是超类还是子类？

使用Pull up Field (320)和Pull up Method (322)，或者Push Down Method (328)和Push Down Field (329)，把想要移除的类的所有行为和数据搬移到另一个类。

每次移动后，编译并测试。

调整即将被移除的那个类的所有引用点，令它们改而引用合并后留下的类。这个动作将会影响变量的声明、参数的类型以及构造函数。

移除我们的目标；此时的它应该已经成为一个空类。

编译，测试。

---

## 11.10 Form Template Method（塑造模板函数）

你有一些子类，其中相应的某些函数以相同顺序执行类似的操作，但各个操作的细节上有所不同。

将这些操作分别放进独立函数中，并保持它们都有相同的签名，于是原函数也就变得相同了。然后将原函数上移至超类。

动机

继承是避免重复行为的一个强大工具。无论何时，只要你看见两个子类之中有类似的函数，就可以把它们提升到超类。但是如果这些函数并不完全相同该怎么办？我们仍有必要尽量避免重复，但又必须保持这些函数之间的实质差异。

常见的一种情况是：两个函数以相同顺序执行大致相近的操作，但是各操作不完全相同。这种情况下我们可以将执行操作的序列移至超类，并借助多态保证各操作仍得以保持差异性。这样的函数被称为Template Method（模板函数）[Gang of Four]。

做法

在各个子类中分解目标函数，使分解后的各个函数要不完全相同，要不完全不同。

运用Pull Up Method (322)将各子类内完全相同的函数上移至超类。

对于那些（剩余的、存在于各子类内的）完全不同的函数，实施Rename Method (273)，使所有这些函数的签名完全相同。

这将使得原函数变为完全相同，因为它们都执行同样一组函数调用；但各子类会以不同方式响应这些调用。

修改上述所有签名后，编译并测试。

运用Pull Up Method (322)将所有原函数逐一上移至超类。在超类中将那些代表各种不同操作的函数定义为抽象函数。

编译，测试。

移除其他子类中的原函数，每删除一个，编译并测试。

范例

现在我将完成第1章遗留的那个范例。在此范例中，我有一个Customer，其中有两个用于打印的函数。statement()函数以ASCII码打印报表：

public String statement(){

Enumeration rentals = _rentals.elements();

String result = "Rental Record for " + getName()+ "\n";

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

// show figures for this rental

result += "\t" + each.getMovie().getTitle()+ "\t" +

String.valueOf(each.getCharge())+ "\n";

}

// add footer lines

result += "Amount owed is " + String.valueOf(getTotalCharge())+ "\n";

result += "You earned "+String.valueOf(getTotalFrequentRenterPoints())

+ " frequent renter points";

return result;

}

函数htmlStatement()则以HTML格式输出报表：

public String htmlStatement(){

Enumeration rentals = _rentals.elements();

String result = "<H1>Rentals for <EM>" + getName()+ "</EM></H1><P>\n";

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

// show figures for each rental

result += each.getMovie().getTitle()+ ": " +

String.valueOf(each.getCharge())+ "<BR>\n";

}

// add footer lines

result += "<P>You owe <EM>" + String.valueOf(getTotalCharge())+

"</EM><P>\n";

result += "On this rental you earned <EM>" +

String.valueOf(getTotalFrequentRenterPoints())+

"</EM> frequent renter points<P>";

return result;

}

使用Form Template Method (345)之前，我需要对上述两个函数做一些整理，使它们成为同一个超类下的子类函数。为了这一目的，我使用函数对象 [Beck]针对“报表打印”创建一个独立的策略继承体系，如图11-1所示。

图11-1 针对“报表输出”使用Strategy模式

class Statement {}

class TextStatement extends Statement {}

class HtmlStatement extends Statement {}

现在，通过Move Method (142)，我将两个负责输出报表的函数分别搬移到对应的子类中：

class Customer...

public String statement(){

return new TextStatement().value(this);

}

public String htmlStatement(){

return new HtmlStatement().value(this);

}

class TextStatement {

public String value(Customer aCustomer){

Enumeration rentals = aCustomer.getRentals();

String result = "Rental Record for " + aCustomer.getName()+ "\n";

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

// show figures for this rental

result += "\t" + each.getMovie().getTitle()+ "\t"

+ String.valueOf(each.getCharge())+ "\n";

}

// add footer lines

result += "Amount owed is "

+ String.valueOf(aCustomer.getTotalCharge())+ "\n";

result += "You earned "

+ String.valueOf(aCustomer.getTotalFrequentRenterPoints())

+ " frequent renter points";

return result;

}

class HtmlStatement {

public String value(Customer aCustomer){

Enumeration rentals = aCustomer.getRentals();

String result = "<H1>Rentals for <EM>" + aCustomer.getName()

+ "</EM></H1><P>\n";

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

// show figures for each rental

result += each.getMovie().getTitle()+ ": "

+ String.valueOf(each.getCharge())+ "<BR>\n";

}

// add footer lines

result += "<P>You owe <EM>"

+ String.valueOf(aCustomer.getTotalCharge())+ "</EM><P>\n";

result += "On this rental you earned <EM>"

+ String.valueOf(aCustomer.getTotalFrequentRenterPoints())

+ "</EM> frequent renter points<P>";

return result;

}

搬移之后，我还对这两个函数的名称做了一些修改，使它们更好地适应Strategy模式的要求。我之所以为它们取相同名称，因为两者之间的差异不在于函数，而在于函数所属的类。如果你想试着编译这段代码，还必须在Customer类中添加一个getRentals()函数，并放宽getTotalCharge()函数和getTotalFrequent-RenterPoints()函数的可见度。

面对两个子类中的相似函数，我可以开始实施Form Template Method (345)了。本重构的关键在于：运用Extract Method (110)将两个函数的不同部分提炼出来，从而将相似的代码和变动的代码分开。每次提炼后，我就建立一个签名相同但本体不同的函数。

第一个例子就是打印报表表头。上述两个函数都通过Customer对象获取信息，但对运算结果字符串的格式化方式不同。我可以将“对字符串的格式化”提炼到独立函数中，并将提炼所得命以相同的签名：

class TextStatement...

String headerString(Customer aCustomer){

return "Rental Record for " + aCustomer.getName()+ "\n";

}

public String value(Customer aCustomer){

Enumeration rentals = aCustomer.getRentals();

String result = headerString(aCustomer);

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

// show figures for this rental

result += "\t" + each.getMovie().getTitle()+ "\t"

+ String.valueOf(each.getCharge())+ "\n";

}

// add footer lines

result += "Amount owed is "

+ String.valueOf(aCustomer.getTotalCharge())+ "\n";

result += "You earned "

+ String.valueOf(aCustomer.getTotalFrequentRenterPoints())

+ " frequent renter points";

return result;

}

class HtmlStatement...

String headerString(Customer aCustomer){

return "<H1>Rentals for <EM>" + aCustomer.getName()+ "</EM></H1><P>\n";

}

public String value(Customer aCustomer){

Enumeration rentals = aCustomer.getRentals();

String result = headerString(aCustomer);

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

// show figures for each rental

result += each.getMovie().getTitle()+ ": "

+ String.valueOf(each.getCharge())+ "<BR>\n";

}

// add footer lines

result += "<P>You owe <EM>"

+ String.valueOf(aCustomer.getTotalCharge())+ "</ EM><P>\n";

result += "On this rental you earned <EM>"

+ String.valueOf(aCustomer.getTotalFrequentRenterPoints())

+ "</EM> frequent renter points<P>";

return result;

}

编译并测试，然后继续处理其他元素。我将逐一对各个元素进行上述过程。下面是整个重构完成后的结果：

class TextStatement ...

public String value(Customer aCustomer){

Enumeration rentals = aCustomer.getRentals();

String result = headerString(aCustomer);

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

result += eachRentalString(each);

}

result += footerString(aCustomer);

return result;

}

String eachRentalString(Rental aRental){

return "\t" + aRental.getMovie().getTitle()+ "\t"

+ String.valueOf(aRental.getCharge())+ "\n";

}

String footerString(Customer aCustomer){

return "Amount owed is " + String.valueOf(aCustomer.getTotalCharge())

+ "\n" + "You earned "

+ String.valueOf(aCustomer.getTotalFrequentRenterPoints())

+ " frequent renter points";

}

class HtmlStatementÖ

public String value(Customer aCustomer){

Enumeration rentals = aCustomer.getRentals();

String result = headerString(aCustomer);

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

result += eachRentalString(each);

}

result += footerString(aCustomer);

return result;

}

String eachRentalString(Rental aRental){

return aRental.getMovie().getTitle()+ ": "

+ String.valueOf(aRental.getCharge())+ "<BR>\n";

}

String footerString(Customer aCustomer){

return "<P>You owe <EM>" + String.valueOf(aCustomer.getTotalCharge())

+ "</EM><P>" + "On this rental you earned <EM>"

+ String.valueOf(aCustomer.getTotalFrequentRenterPoints())

+ "</EM> frequent renter points<P>";

}

所有这些修改都完成后，两个value()函数看上去已经非常相似了，因此我可以使用Pull up Method (322)将它们提升到超类中。提升完毕后，我需要在超类中把子类函数声明为抽象函数。

class Statement...

public String value(Customer aCustomer){

Enumeration rentals = aCustomer.getRentals();

String result = headerString(aCustomer);

while (rentals.hasMoreElements()){

Rental each = (Rental)rentals.nextElement();

result += eachRentalString(each);

}

result += footerString(aCustomer);

return result;

}

abstract String headerString(Customer aCustomer);

abstract String eachRentalString(Rental aRental);

abstract String footerString(Customer aCustomer);

然后我把TextStatement.value()函数拿掉，编译并测试。完成之后再把HtmlStatement.value()也删掉，再次编译并测试。最后结果如图11-2所示。

完成本重构后，处理其他种类的报表就容易多了：你只需为Statement再建一个子类，并在其中覆写3个抽象函数即可。

图11-2 Template Method（模板函数）塑造完毕后的类

---

## 11.11 Replace Inheritance with Delegation （以委托取代继承）

某个子类只使用超类接口中的一部分，或是根本不需要继承而来的数据。

在子类中新建一个字段用以保存超类；调整子类函数，令它改而委托超类；然后去掉两者之间的继承关系。

动机

继承是个好东西，但有时候它并不是你要的。你常常会遇到这样的情况：一开始继承了一个类，随后发现超类中的许多操作并不真正适用于子类。这种情况下，你所拥有的接口并未真正反映出子类的功能。或者，你可能发现你从超类中继承了一大堆子类并不需要的数据，抑或你可能发现超类中的某些protected函数对子类并没有什么意义。

你可以选择容忍，并接受传统说法：子类可以只使用超类功能的一部分。但这样做的结果是：代码传达的信息与你的意图南辕北辙——这是一种混淆，你应该将它去除。

如果以委托取代继承，你可以更清楚地表明：你只需要受托类的一部分功能。接口中的哪一部分应该被使用，哪一部分应该被忽略，完全由你主导控制。这样做的成本则是需要额外写出委托函数，但这些函数都非常简单，极少可能出错。

做法

在子类中新建一个字段，使其引用超类的一个实例，并将它初始化为this。

修改子类内的所有函数，让它们不再使用超类，转而使用上述那个受托字段。

每次修改后，编译并测试。

你不能这样修改子类中通过super调用超类函数的代码，否则它们会陷入无限递归。这种函数只有在继承关系被打破后才能修改。

去除两个类之间的继承关系，新建一个受托类的对象赋给受托字段。

针对客户端所用的每一个超类函数，为它添加一个简单的委托函数。

编译，测试。

范例

滥用继承的一个经典范例就是让Stack类继承Vector类──Java 1.1的工具库（java.util）恰好就是这样做的。（这些淘气的孩子啊!）不过，作为范例，我只给出一个比较简单的形式：

class MyStack extends Vector {

public void push(Object element){

insertElementAt(element,0);

}

public Object pop(){

Object result = firstElement();

removeElementAt(0);

return result;

}

}

只要看看MyStack的用户，我就会发现，用户只要它做4件事：push()、pop()、size()和isEmpty()。后两个函数是从vector继承来的。

我要把这里的继承关系改为委托关系。首先，我要在MyStack中新建一个字段，用以保存受托的Vector对象。一开始我把这个字段初始化为this，这样在重构进行过程中，我就可以同时使用继承和委托：

private Vector _vector = this;

现在，我开始修改MyStack的函数，让它们使用委托关系。首先从push()开始：

public void push(Object element){

_vector.insertElementAt(element,0);

}

此时我可以编译并测试，一切都将运转如常。现在轮到pop()：

public Object pop(){

Object result = _vector.firstElement();

_vector.removeElementAt(0);

return result;

}

修改完所有子类函数后，我可以打破与超类之间的联系了：

class MyStack extends Vector

private Vector _vector = new Vector();

然后，对于Stack客户端可能用到的每一个Vector函数 ，我都必须在MyStack中添加一个简单的委托函数：

public int size(){

return _vector.size();

}

public boolean isEmpty(){

return _vector.isEmpty();

}

现在我可以编译并测试。如果我忘记加入某个委托函数，编译器会告诉我。

---

## 11.12 Replace Delegation with Inheritance （以继承取代委托）

你在两个类之间使用委托关系，并经常为整个接口编写许多极简单的委托函数。

让委托类继承受托类。

动机

本重构与Replace Inheritance with Delegation (352)恰恰相反。如果你发现自己需要使用受托类中的所有函数，并且费了很大力气编写所有极简的委托函数，本重构可以帮助你轻松回头使用继承。

两条告诫需牢记于心。首先，如果你并没有使用受托类的所有函数，那么就不应该使用Replace Delegation With Inheritance (355)，因为子类应该总是遵循超类的接口。如果过多的委托函数让你烦心，你有别的选择：你可以通过Remove Middle Man (160)让客户端自己调用受托函数，也可以使用Extract Superclass (336)将两个类接口相同的部分提炼到超类中，然后让两个类都继承这个新的超类；你还可以用类似的手法使用Extract Interface (341)。

另一种需要当心的情况是：受托对象被不止一个其他对象共享，而且受托对象是可变的。在这种情况下，你就不能将委托关系替换为继承关系，因为这样就无法再共享数据了。数据共享是必须由委托关系承担的一种责任，你无法把它转给继承关系。如果受托对象是不可变的，数据共享就不成问题，因为你大可放心地复制对象，谁都不会知道。

做法

让委托端成为受托端的一个子类。

编译。

此时，某些函数可能会发生冲突：它们可能有相同的名称，但在返回类型、异常指定或可见程度方面有所差异。你可以使用Remane Method (273)解决此类问题。

将受托字段设为该字段所处对象本身。

去掉简单的委托函数。

编译并测试。

将所有其他涉及委托关系的代码，改为调用对象自身。

移除受托字段。

范例

下面是一个简单的Employee类，将一些函数委托给另一个同样简单的Person类：

class Employee {

Person _person = new Person();

public String getName(){

return _person.getName();

}

public void setName(String arg){

_person.setName(arg);

}

public String toString(){

return "Emp: " + _person.getLastName();

}

}

class Person {

String _name;

public String getName(){

return _name;

}

public void setName(String arg){

_name = arg;

}

public String getLastName(){

return _name.substring(_name.lastIndexOf(' ')+ 1);

}

}

第一步，只需声明两者之间的继承关系：

class Employee extends Person

此时，如果有任何函数发生冲突，编译器会提醒我。如果某几个函数的名称相同，但返回类型不同，或抛出不同的异常，它们之间就会出现冲突。所有此类问题都可以通过Rename Method (273)加以解决。为求简化，我没有在范例中列出这些麻烦情况。

下一步要将受托字段设值为该字段所处对象自身。同时，我必须先删掉所有简单的委托函数（例如getName()和setName()）。如果留下这种函数，就会因为无限递归而引起系统调用栈溢出。在此范例中，我应该把Employee的getName()和setName()拿掉。

一旦Employee可以正常工作了，我就修改其中用到委托函数的代码，让它们直接调用从超类继承而来的函数：

public String toString (){

return "Emp: " + getLastName();

}

摆脱所有涉及委托关系的函数后，我也就可以摆脱_person这个受托字段了。

---
