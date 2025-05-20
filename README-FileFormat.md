## EcoViz File Formats

Vegetation data (i.e., trees or cohorts of trees) are loaded in EcoViz from data files with a special format. The format specification is given below.

Note that EcoViz supports a text-file-based format (PDB) and a binary format (PDBB). The latter is decreasing file size and increasing performance substantially. 


todo: species codes, AAIgrid, space (origin)
-----

### Text-Based PDB File Format (`.pdb`)

This format is human-readable and stores data as text.

| Description                     | Data Type (in C++ context) | Example                                                | Notes                                                                 |
| :------------------------------ | :------------------------- | :----------------------------------------------------- | :-------------------------------------------------------------------- |
| PDB File Version                | String                     | `"3.0"`                                                | Indicates the version of the PDB file format.                         |
| World Origin X                  | Double                     | `253323`                                                 | The X-coordinate of the model's world origin in metric coordinates. |
| World Origin Y                  | Double                     | `20353`                                                | The Y-coordinate of the model's world origin in metric coordinates. |
| Time Step Number                | Integer                    | `1`                                                    | The simulation year for which the data is recorded.                   |
| Total Number of Trees           | Integer                    | `100000`                                              | Placeholder for the total count of trees, written after tree data.  |
| **Tree Data (repeated for each tree, one line per tree)** |                            |                                                        |                                                                       |
| Tree ID                         | Integer                    | 1                                           | Unique identifier for the tree.                                       |
| Species ID                      | Char                       | `piab`                                                | Identifier for the tree species.                                      |
| Position X                      | Float                      | `100`                                                 | X-coordinate of the tree (m)  (relative to origin)                                        |
| Position Y                      | Float                      | `35.5`                                                 | Y-coordinate of the tree (m) (relative to origin)                                            |
| Height                          | Float                      | `20.7`                                       | Height of the tree (m)                                                  |
| Canopy Radius                   | Float                      | `3.23`               | Radius of the tree's canopy (m)                                       |
| Diameter at Breast Height (DBH) | Float                      | `22.1`                                          | Diameter of the tree at breast height (cm)                                |
| Status                          | Integer                    | `0`                             | Tree status: 1 if dead, 0 if alive.                                   |
| Total Number of Saplings (Cohorts) | Integer                    | `100000`                                            | Placeholder for the total count of saplings, written after sapling data. |
| **Cohort Data (repeated for each  cohort, one line per cohort)** |                            |                                                        |                                                                       |
| Position X                      | Float                      | `2.5`                                            | X-coordinate of the center of the cohort cell (m) (relative to origin)                                   |
| Position Y                      | Float                      | `1234`                                            | Y-coordinate of the center of the cohort cell (m) (relative to origin)                                   |
| Species ID                      | Char                       | `lade`                                 | Identifier for the cohort species.                                   |
| DBH                             | Float                      | `0.23`                                                  | DBH of the cohort (cm).                                        |
| Height                          | Float                      | `1.4`                                | Height of the cohort  (m)                                                |
| Represented Stem Number         | Float                      | `23`                                               | Number of trees represented by this cohort.                   |

**Note:** Fields within a line are separated by a whitespace character (typically a blank, ' '). Each data entry (version, origin, timestep, tree record, cohort record) is on a new line.


Example of a PDB file:
```
3.0
4570108 5269625
1
423947    
4109621 lade 595 97 15.0974 2.5 27.4414 0
....
```

-----

### Binary PDB File Format (`.pdbb`)

This format is a more compact, binary representation of the data. The filename has a 'b' appended to the user-defined pattern (e.g., `output/year1.pdbb`).

| Description                      | C++ Data Type          | Size (Bytes) | Notes                                                                                                |
| :------------------------------- | :--------------------- | :----------- | :--------------------------------------------------------------------------------------------------- |
| Length of Version String         | `int`                  | 4            | The number of characters in the version string (e.g., for "3.0", this would be 3).                      |
| Version String                   | `char[]`               | `slen`       | The PDB file version string itself (e.g., "3.0"). Does not store a null terminator.                  |
| World Origin X                   | `int64_t`              | 8            | The X-coordinate of the model's world origin.                                                        |
| World Origin Y                   | `int64_t`              | 8            | The Y-coordinate of the model's world origin.                                                        |
| Time Step Number                 | `int`                  | 4            | The simulation year.                                                                                 |
| Total Number of Trees            | `int`                  | 4            | The total count of individual trees.                                                                 |
| **Tree Data Block (Part A)** | `cohortA[]`            | `n_trees` \* sizeof(`cohortA`) | A contiguous block of `cohortA` structures.                                                         |
|    Tree ID (not explicitly stored, implied by order) | `(int)`              | (0)        | The `treeid` field in `cohortA` struct is declared but not explicitly written/used from iLand tree ID. |
|    Species Code   | `char[4]`              | 4            | 4-byte ASCII code for the tree species, 0-padded.                                                      |
|    Position X     | `int`                  | 4            | X-coordinate of the tree.                                                                              |
|    Position Y     | `int`                  | 4            | Y-coordinate of the tree.                                                                              |
|    Height         | `float`                | 4            | Height of the tree.                                                                                    |
|    Canopy Radius  | `float`                | 4            | Radius of the tree's canopy.                                                                           |
|    DBH            | `float`                | 4            | Diameter of the tree at breast height.                                                                 |
|    Status (Dummy) | `int`                  | 4            | Tree status: 1 if dead, 0 if alive.                                                                    |
| Total Number of Sapling Cohorts  | `int`                  | 4            | The total count of sapling cohorts.                                                                  |
| **Sapling Data Block (Part B)** | `cohortB[]`            | `n_cohorts` \* sizeof(`cohortB`) | A contiguous block of `cohortB` structures.                                                        |
|    Position X     | `int`                  | 4            | X-coordinate of the sapling cohort.                                                                  |
|    Position Y     | `int`                  | 4            | Y-coordinate of the sapling cohort.                                                                  |
|    Species Code   | `char[4]`              | 4            | 4-byte ASCII code for the sapling species, 0-padded.                                                   |
|    DBH            | `float`                | 4            | DBH of the sapling cohort.                                                                             |
|    Height         | `float`                | 4            | Height of the sapling cohort.                                                                          |
|    Number of Plants | `float`                | 4            | Number of plants represented by this sapling cohort.                                                   |

**Struct Definitions from Code (for clarity of binary format):**

