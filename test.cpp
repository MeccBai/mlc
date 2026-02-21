//
// Created by Administrator on 2026/2/21.
//

import Parser;
import std;
import Prepare;
int main() {
    mlc::parser::AbstractSyntaxTree ast({});
    std::string content = "int x, y = 10, *p = &x, a[10] = {0,1,2}, b[10] = {3,4,5}, *xp = a";

    auto result = mlc::prepare::Prepare(content);
    ast.variableParser(result);


    auto r2 = ast.expressionParser("(a->x+b.t)*(c+d)");
}
