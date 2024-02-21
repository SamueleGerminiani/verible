import json
import sys

def generate_graphviz(children, parent_id):
    for i, node in enumerate(children):
        node_id = f"{parent_id}_{i}"
        label = ""
        attributes = ""

        if "tag" in node:
            tag = node["tag"]
            label = f"{tag}"
            if "text" in node:
                text = node["text"]
                label = f"{label} : {text}"

        if not "children" in node:
            attributes = "shape=box"

        print(f'{node_id} [label="{label}" {attributes}];')

        if parent_id != "":
            print(f'{parent_id} -> {node_id};')

        if "children" in node:
            generate_graphviz(node["children"], node_id)

def main(json_file):
    with open(json_file, "r") as file:
        json_data = json.load(file)

    root_node = list(json_data.keys())[0]
    graph_name = "tree"

    print(f"digraph {graph_name} {{")
    print("node [shape=ellipse];")

    first_child = json_data[root_node]["tree"]

    tag = first_child["tag"]
    print(f'x [label="{tag}"];')
    generate_graphviz(first_child["children"], "x")



    print("}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 script.py <json_file>")
        sys.exit(1)

    main(sys.argv[1])

