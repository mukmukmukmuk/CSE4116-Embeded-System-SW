# Project#4 - How to Create and Submit a Kernel Patch
*Sogang University – Embedded Systems Software (CSE4116, Prof. Youngjae Kim)*

---

This document explains **how to manage your modified kernel source with Git and how to submit your work as a single patch file (`.patch`)**.

> You must follow this workflow from the beginning of the project.  
> Do **NOT** submit the whole kernel source tree.

---

## What Is a Patch File?

A patch file is a text file that contains the differences between:
- the original Linux kernel source, and
- your modified kernel source

For this project, you must submit a patch file named:

> `prj4-kernel-{student_id}.patch`
> e.g., prj4-kernel-20241234.patch

---

## Step 1. Initialize Git Immediately After Installing the Kernel Source (One-Time)

Right after you download and extract the Linux kernel tarball, do the following:

```bash
cd ~/linux-5.15.190
git init
git add .
git commit -m "vanilla 5.15.190 baseline"
git tag baseline
````

This first commit is your clean baseline (unmodified kernel). This records the original kernel source as the baseline. All your later changes will be measured relative to this commit.

Note (About Git user identity):
* When making the first commit, Git may ask for your user name and email address. 
* If you do not want to configure your real Git account or use a global Git identity, you can set a temporary identity for this repository only as follows (`student_id` should be yours):

```bash
git config user.name "20211496"
git config user.email "ksm010825@sogang.ac.kr"
````

* This configuration applies only to this kernel source directory and does not affect any other repositories or system-wide Git settings.
* Do not use --global in this lab environment.

Important:
* **You must create this baseline commit before you start modifying any files**.
* And, don't be confused. `git init` takes a long time for sure.

---

## Step 2. Develop and (Optionally) Commit as You Work

During development, you can freely edit files, build, and test.
You _may (not necessarily)_ create additional commits if you like, for example:

```bash
# After some changes
git status                 # optional, just to see modified files
git add <modified files>   # or: git add .
git commit -m "implement OpenSSD mkfs support (first attempt)"

# After more fixes
git add <more files>
git commit -m "fix bug in nvme driver"
````

This step is flexible, and doesn't matter at all if you correctly tag the baseline before:
* **Only the final diff between baseline and your last commit matters for grading**.

---

## Step 3. Prepare Your Final State Before Submission (One-Time)

When you are done:
1. Make sure your code builds and boots as required.
2. Ensure your working tree is clean:

```bash
cd ~/linux-5.15.190
git status
````

If you still see "modified" files, commit them:

```bash
git add .
git commit -m "final version for submission"
````

Now git status should show:

> nothing to commit, working tree clean

---

## Step 4. Generate and Submit the Patch File (One-Time)

You will submit one patch file that contains all changes from the baseline to your final version.

```bash
git diff baseline -- > prj4-kernel-20211496.patch
````

Replace `{student_id}` with your student ID (e.g., 20241234).

This generates a single patch file that includes:
* All diffs against the previous commit (which should be your working baseline or prior revision)

Important rule for submission:
* **Submit only this .patch file, not the entire kernel tree.**

---

## Tips

Make sure that:
* Only files you intentionally modified appear
* No compiled files (.o, .ko, etc.) are included
* The patch clearly reflects your intended fix

Do NOT submit:
* Linux kernel binaries (vmlinuz-*)
* `/lib/modules/*`
* The entire kernel source tree
* Compiled objects or temporary files

Important Notes:
* The grader will inspect your patch file only.

