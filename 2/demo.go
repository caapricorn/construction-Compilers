package main

import (
	"fmt"
	"go/ast"
	"go/format"
	"go/parser"
	"strconv"
	"go/token"
	"os"
)

var x = 0

func insertIntVar(file *ast.File, name string, value int) {
	var before, after []ast.Decl

	if len(file.Decls) > 0 {
		hasImport := false
		if genDecl, ok := file.Decls[0].(*ast.GenDecl); ok {
			hasImport = genDecl.Tok == token.IMPORT
		}

		if hasImport {
			before, after = []ast.Decl{file.Decls[0]}, file.Decls[1:]
		} else {
			after = file.Decls
		}
	}

	file.Decls = append(before,
		&ast.GenDecl{
			Tok: token.VAR,
			Specs: []ast.Spec{
				&ast.ValueSpec{
					Names: []*ast.Ident{ast.NewIdent(name)},
					Type:  ast.NewIdent("int"),
					Values: []ast.Expr{
						&ast.BasicLit{
							Kind:  token.INT,
							Value: fmt.Sprintf("%d", value),
						},
					},
				},
			},
		},
	)
	file.Decls = append(file.Decls, after...)
}

func insertCounters(file *ast.File) {
	ast.Inspect(file, func(node ast.Node) bool {
		if blockStmt, ok := node.(*ast.BlockStmt); ok {
			ind := 0
			for i := 0; i < len(blockStmt.List); {
				val := blockStmt.List[i]
				if forStmt, ok := val.(*ast.ForStmt); ok {
					insertIntVar(file, "i"+generateName(), 0)
					ind = i
					forStmt.Body.List = append(
						[]ast.Stmt{
							&ast.AssignStmt{
								Lhs: []ast.Expr{
									&ast.Ident{
										Name: "i" + strconv.Itoa(x),
										Obj:  nil,
									},
								},
								Tok: token.ADD_ASSIGN,
								Rhs: []ast.Expr{
									&ast.BasicLit{
										Kind:  token.INT,
										Value: "1",
									},
								},
							},
						},
						forStmt.Body.List...,
					)
					blockStmt.List = append(append(blockStmt.List[:ind], forStmt), blockStmt.List[ind:]...)
					onePart := append(blockStmt.List[:ind+1], []ast.Stmt{
						&ast.ExprStmt{
							X: &ast.CallExpr{
								Fun: &ast.SelectorExpr{
									X:   ast.NewIdent("fmt"),
									Sel: ast.NewIdent("Printf"),
								},
								Args: []ast.Expr{
									&ast.BasicLit{
										Kind:  token.STRING,
										Value: "\"%d\"",
									},
									&ast.Ident{
										Name: "i" + strconv.Itoa(x),
									},
								},
							},
						},
					}...)
					blockStmt.List = append(onePart, blockStmt.List[ind+2:]...)
				}
				i += 1
			}

		}
		return true
	})
}

func generateName() string {
	x = x + 1
	return strconv.Itoa(x)
}

func main() {
	if len(os.Args) != 2 {
		return
	}

	fset := token.NewFileSet()
	if file, err := parser.ParseFile(fset, os.Args[1], nil, parser.ParseComments); err == nil {
		//insertIntVar(file, "xxx", 666)
		//insertHello(file)
		insertCounters(file)

		if format.Node(os.Stdout, fset, file) != nil {
			fmt.Printf("Formatter error: %v\n", err)
		}
		ast.Fprint(os.Stdout, fset, file, nil)
	} else {
		fmt.Printf("Errors in %s\n", os.Args[1])
	}
}
