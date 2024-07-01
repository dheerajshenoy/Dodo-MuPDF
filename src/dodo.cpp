#include "dodo.hpp"
#include <mupdf/fitz/geometry.h>
#include <mupdf/fitz/write-pixmap.h>

Dodo::Dodo(int argc, char** argv, QWidget *parent)
{
    //m_layout->addWidget(m_label);
    m_layout->addWidget(m_scrollarea);
    m_layout->addWidget(m_statusbar);
    m_scrollarea->setWidget(m_label);

    m_widget->setLayout(m_layout);
    m_scrollarea->setAlignment(Qt::AlignmentFlag::AlignCenter);
    m_label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    m_scrollarea->setWidgetResizable(true);
    this->setCentralWidget(m_widget);

    INIT_PDF();
    SetKeyBinds();
    this->show();

    if (argc > 1)
        Open(QString(argv[1]));
}

void Dodo::SetKeyBinds()
{
    QShortcut *kb_zoomin = new QShortcut(QKeySequence("="), this, [&]() {
        this->Zoom(+10);
    });

    QShortcut *kb_zoomout = new QShortcut(QKeySequence("-"), this, [&]() {
        this->Zoom(-10);
    });

    QShortcut *kb_zoomreset = new QShortcut(QKeySequence("+"), this, [&]() {
        this->ZoomReset();
    });

    QShortcut *kb_open = new QShortcut(QKeySequence("Ctrl+o"), this, [&]() {
        QString filename = QFileDialog::getOpenFileName(this, "Open file", nullptr, "PDF Files (*.pdf);;");
        this->Open(filename);
    });

    QShortcut *kb_next_page = new QShortcut(QKeySequence("Shift+j"), this, [&]() {
        this->GotoPage(1);
    });


    QShortcut *kb_prev_page = new QShortcut(QKeySequence("Shift+k"), this, [&]() {
        this->GotoPage(-1);
    });

    QShortcut *kb_scroll_down = new QShortcut(QKeySequence("j"), this, [&]() {
        this->ScrollVertical(1);
    });

    QShortcut *kb_scroll_up = new QShortcut(QKeySequence("k"), this, [&]() {
        this->ScrollVertical(-1);
    });

    QShortcut *kb_scroll_left = new QShortcut(QKeySequence("h"), this, [&]() {
        this->ScrollHorizontal(-1);
    });

    QShortcut *kb_scroll_right = new QShortcut(QKeySequence("l"), this, [&]() {
        this->ScrollHorizontal(1);
    });

    QShortcut *kb_rotate_clock = new QShortcut(QKeySequence(","), this, [&]() {
        this->Rotate(90);
    });


    QShortcut *kb_rotate_anticlock = new QShortcut(QKeySequence("."), this, [&]() {
        this->Rotate(-90);
    });

    QShortcut *kb_reset_view = new QShortcut(QKeySequence("Shift+R"), this, [&]() {
        this->ResetView();
    });

}

void Dodo::GotoPage(int pinterval)
{
    // Check for out of bounds
    if (m_page_number + pinterval > m_page_count - 1 ||
        m_page_number + pinterval < 0)
    {
        return;
    }

    m_page_number += pinterval;
    m_statusbar->SetCurrentPage(m_page_number);
    Render();
}

void Dodo::Render()
{

    m_ctm = fz_scale(m_zoom / 100, m_zoom / 100);
    m_ctm = fz_pre_rotate(m_ctm, m_rotate);

    // Render page to an RGB pixmap.
    fz_try(m_ctx)
    {
        m_pix = fz_new_pixmap_from_page_number(m_ctx, m_doc, m_page_number, m_ctm, fz_device_rgb(m_ctx), 1);
        QImage img(m_pix->samples, m_pix->w, m_pix->h, QImage::Format_RGBA8888, nullptr, m_pix->samples);
        m_label->setPixmap(QPixmap::fromImage(img));
    }
    fz_catch(m_ctx)
    {
        fprintf(stderr, "cannot render page: %s\n", fz_caught_message(m_ctx));
        fz_drop_document(m_ctx, m_doc);
        fz_drop_context(m_ctx);
        return;
    }
}
void Dodo::ZoomReset()
{
    m_zoom = 100.0f;
    Render();
}

void Dodo::Zoom(float r)
{
    if (m_zoom + r <= 0) return;

    m_zoom += r;
    Render();
}

bool Dodo::INIT_PDF()
{
    return true;
}

bool Dodo::Open(QString filename, int page_number)
{
    m_ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    
    /* Create a context to hold the exception stack and various caches. */
    if (!m_ctx)
    {
        fprintf(stderr, "cannot create mupdf context\n");
        qFatal("Cannot create mupdf context");
        return false;
    }

    /* Register the default file types to handle. */
    fz_try(m_ctx)
    {
        fz_register_document_handlers(m_ctx);
    }
    fz_catch(m_ctx)
    {
        fz_report_error(m_ctx);
        fprintf(stderr, "cannot register document handlers: %s", fz_caught_message(m_ctx));
        qFatal("Cannot register document handlers");
        fz_drop_context(m_ctx);
        return false;
    }
    
    m_filename = QString::fromStdString(filename.toStdString());

    fz_try(m_ctx)
    {
        m_doc = fz_open_document(m_ctx, m_filename.toStdString().c_str());
    }

    fz_catch(m_ctx)
    {
        fprintf(stderr, "cannot open document: %s\n", fz_caught_message(m_ctx));
        fz_drop_context(m_ctx);
        fz_drop_document(m_ctx, m_doc);
        return false;
    }

    // Count the number of pages
    fz_try(m_ctx)
    {
        m_page_count = fz_count_pages(m_ctx, m_doc);
    }
    fz_catch(m_ctx)
    {
        fprintf(stderr, "cannot count number of pages: %s\n", fz_caught_message(m_ctx));
        fz_drop_document(m_ctx, m_doc);
        fz_drop_context(m_ctx);
        return false;
    }

    if (page_number < 0 || page_number > m_page_count)
    {
        fprintf(stderr, "incorrect page number");
        fz_drop_document(m_ctx, m_doc);
        fz_drop_context(m_ctx);
        return false;
    }

    m_page_number = page_number;

    /* Compute a transformation matrix for the zoom and rotation desired. */
    /* The default resolution without scaling is 72 dpi. */
    m_ctm = fz_scale(m_zoom / 100, m_zoom / 100);
    m_ctm = fz_pre_rotate(m_ctm, m_rotate);

    /* Render page to an RGB pixmap. */
    fz_try(m_ctx)
    {
        /*m_pix = fz_new_pixmap_from_page_number(m_ctx, m_doc, m_page_number, m_ctm, fz_device_rgb(m_ctx), 0);*/
        fz_page *page = fz_load_page(m_ctx, m_doc, 0);
    
        if (!page)
        {
    
            fz_drop_page(m_ctx, page);
            exit(0);
        }

        m_ctm = fz_scale(m_zoom / 100.0f, m_zoom / 100.0f);
        m_ctm = fz_rotate(m_rotate);

        // get transformed page size

        m_pix = fz_new_pixmap_from_page_number(m_ctx, m_doc, m_page_number, m_ctm, fz_device_rgb(m_ctx), 1);

        m_image = QImage(m_pix->samples, m_pix->w, m_pix->h, QImage::Format_RGBA8888);
        m_label->setPixmap(QPixmap::fromImage(m_image));
        m_label->resize(m_label->sizeHint());
    }
    fz_catch(m_ctx)
    {
        fprintf(stderr, "cannot render page: %s\n", fz_caught_message(m_ctx));
        fz_drop_document(m_ctx, m_doc);
        fz_drop_context(m_ctx);
        return false;
    }

    m_statusbar->SetFileName(m_filename);
    m_statusbar->SetFilePageCount(m_page_count);
    m_statusbar->SetCurrentPage(page_number);

    return true;
}

void Dodo::Rotate(float angle)
{
    m_rotate += angle;
    m_rotate=(float)((int)m_rotate % 360); 

    this->Render();
}

void Dodo::ResetView()
{
    m_rotate = 0.0f;
    m_zoom = 100.0f;

    this->Render();
}

void Dodo::ScrollVertical(int direction)
{
    if (direction == 1)
    {
        m_vscroll->setValue(m_vscroll->value() + 30);
        /*m_scrollarea->scroll(0, -m_scroll_len);*/
    } else {
        m_vscroll->setValue(m_vscroll->value() - 30);
    }
}

void Dodo::ScrollHorizontal(int direction)
{

    if (direction == 1)
    {
        m_hscroll->setValue(m_hscroll->value() + 30);
        /*m_scrollarea->scroll(0, -m_scroll_len);*/
    } else {
        m_hscroll->setValue(m_hscroll->value() - 30);
    }
}


Dodo::~Dodo()
{
    /* Clean up. */

    if (m_pix)
        fz_drop_pixmap(m_ctx, m_pix);

    if (m_doc)
        fz_drop_document(m_ctx, m_doc);

    if (m_ctx)
        fz_drop_context(m_ctx);

}
